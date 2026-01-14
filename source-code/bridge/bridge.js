import express from 'express';
import net from 'net';
import { WebSocketServer } from 'ws';

// === Express app (HTTP for non-persistent actions) ======================
const app = express();
app.use(express.json());

const HTTP_PORT = 3001;
const WS_PORT = 3002;

// Enable CORS for all routes
app.use((req, res, next) => {
  res.header("Access-Control-Allow-Origin", "*");
  res.header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
  res.header("Access-Control-Allow-Headers", "Content-Type, Authorization");
  if (req.method === "OPTIONS") return res.sendStatus(200);
  next();
});

// === Backend connection details ========================================
const BACKEND_HOST = process.env.TCP_BACKEND_ADDRESS || 'localhost'; // default host = 'localhost'
const BACKEND_PORT = parseInt(process.env.TCP_BACKEND_PORT || '8080'); // default port = '8080'

console.log(`Forwarding to backend on "${BACKEND_HOST}:${BACKEND_PORT}"`);

// === Framing helpers (length-prefixed JSON over TCP) ===================

/**
 * Builds a TCP frame: [4 bytes length (big endian)] + [JSON UTF-8 bytes]
 */
function buildFrame(jsonStr) {
  const body = Buffer.from(jsonStr, 'utf8');
  const header = Buffer.alloc(4);
  header.writeUInt32BE(body.length, 0);
  return Buffer.concat([header, body]);
}

/**
 * Parses framed data from a buffer.
 * Consumes as many complete frames as possible and returns:
 *  { frames: string[], rest: Buffer }
 */
function parseFrames(buffer) {
  const frames = [];
  let offset = 0;

  while (buffer.length - offset >= 4) {
    const len = buffer.readUInt32BE(offset);
    if (buffer.length - offset < 4 + len) break; // incomplete frame

    const jsonBuf = buffer.slice(offset + 4, offset + 4 + len);
    frames.push(jsonBuf.toString('utf8'));

    offset += 4 + len;
  }

  const rest = buffer.slice(offset);
  return { frames, rest };
}

// =======================================================================
// SECTION 1 — NON-PERSISTENT COMMUNICATION (HTTP)
// Used for one-shot actions like signup or password reset
// =======================================================================

// Helper function to communicate with the C backend via TCP (framed JSON)
function sendToBackend(message) {
  return new Promise((resolve, reject) => {
    const client = new net.Socket();
    let buffer = Buffer.alloc(0);
    let resolved = false;

    client.connect(BACKEND_PORT, BACKEND_HOST, () => {
      console.log(`Connected to backend at ${BACKEND_HOST}:${BACKEND_PORT}`);

      // message is already a JSON string
      const frame = buildFrame(message);
      client.write(frame);
    });

    client.on('data', (data) => {
      buffer = Buffer.concat([buffer, data]);
      const parsed = parseFrames(buffer);
      buffer = parsed.rest;

      // For HTTP one-shot we expect only one response frame
      if (parsed.frames.length > 0 && !resolved) {
        const jsonStr = parsed.frames[0];
        console.log(`Backend responded (framed): ${jsonStr}`);
        resolved = true;
        client.end();
        resolve(jsonStr);
      }
    });

    client.on('error', (err) => {
      console.error('TCP error in sendToBackend:', err.message);
      if (!resolved) {
        resolved = true;
        reject(err);
      }
    });

    client.on('close', () => {
      if (!resolved) {
        reject(new Error('Backend closed connection without sending a complete frame'));
      }
    });
  });
}

// HTTP endpoint (signup, etc.)
// When an HTTP request arrives at /api/send the callback is executed
// In req we have all information sent from frontend
app.post('/api/send', async (req, res) => {
  try {
    const message = req.body.message;
    if (!message) return res.status(400).json({ error: 'Missing message field' });

    console.log(`Forwarding one-shot request to backend: ${message}`);
    const response = await sendToBackend(message);
    console.log(`Backend responded: ${response}`);

    // We send HTTP backend response to client
    res.json({ backendResponse: response.trim() });
  } catch (err) {
    console.error('Bridge error:', err);
    res.status(500).json({ error: 'Failed to communicate with backend' });
  }
});

// Start the bridge HTTP server
app.listen(HTTP_PORT, () => {
  console.log(`HTTP Bridge listening on internal port ${HTTP_PORT} (non-persitent session)`);
});


// =======================================================================
// SECTION 2 — PERSISTENT COMMUNICATION (WebSocket)
// Used for logged-in users that need real-time communication
// =======================================================================

// Store active user connections: each WebSocket ↔ TCP socket pair
const userConnections = new Map();

// Define the bridge WebSocket server
const wss = new WebSocketServer({ port: WS_PORT });

wss.on('connection', (ws) => {
  console.log('Frontend connected via WebSocket');

  // Create a TCP socket to backend for this user
  const client = new net.Socket();
  userConnections.set(ws, client);

  // Buffer used to accumulate framed data coming from backend
  let tcpBuffer = Buffer.alloc(0);

  // client represents the TCP connection (with backend)
  // ws represents the WebSocket connection (with frontend)
  client.connect(BACKEND_PORT, BACKEND_HOST, () => {
    console.log(`Connected persistent TCP → ${BACKEND_HOST}:${BACKEND_PORT}`);

    // Print connection table
    console.log(`\n Number of open websocket connections: ${userConnections.size}`);
    console.log("\n=== CURRENT CONNECTIONS ===\n");

    const connectionsInfo = [];

    for (const [wsConn, clientConn] of userConnections) {
      connectionsInfo.push({
        WebSocketState: wsConn.readyState,
        TCP: `${clientConn.remoteAddress}:${clientConn.remotePort}`
      });
    }

    console.table(connectionsInfo);
  });

  // --- FRONTEND → BACKEND ------------------------------------------------
  ws.on('message', (msg) => {
    const jsonStr = msg.toString(); // frontend sends already JSON.stringify(...)
    console.log(`[Frontend → Backend]: ${jsonStr}`);

    const frame = buildFrame(jsonStr);
    client.write(frame);
  });

  // --- BACKEND → FRONTEND ------------------------------------------------
  client.on('data', (data) => {
    tcpBuffer = Buffer.concat([tcpBuffer, data]);

    const parsed = parseFrames(tcpBuffer);
    tcpBuffer = parsed.rest;

    for (const jsonStr of parsed.frames) {
      console.log(`[Backend → Frontend]: ${jsonStr}`);

      // We keep the same wrapper expected by the Angular WebsocketService
      ws.send(JSON.stringify({ backendResponse: jsonStr }));
    }
  });

  // --- CONNECTION MANAGEMENT ---------------------------------------------
  ws.on('close', () => {
    console.log('Frontend disconnected — closing backend TCP');
    client.end();
    userConnections.delete(ws);
  });

  client.on('close', () => {
    console.log('Backend closed connection — closing WebSocket');
    // After the TCP connection with backend is closed, we also close WebSocket connection with frontend
    if (ws.readyState === ws.OPEN) ws.close();
    userConnections.delete(ws);
  });

  client.on('error', (err) => {
    console.error('TCP connection error:', err.message);
    if (ws.readyState === ws.OPEN) {
      ws.send(JSON.stringify({ error: 'TCP connection error: ' + err.message }));
    }
  });
});

console.log(`WebSocket Bridge listening on internal port ${WS_PORT} (persistent sessions)`);
