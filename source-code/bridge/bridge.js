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

// =======================================================================
// SECTION 1 — NON-PERSISTENT COMMUNICATION (HTTP)
// Used for one-shot actions like signup or password reset
// =======================================================================

// Helper function to communicate with the C backend via TCP
function sendToBackend(message) {
  return new Promise((resolve, reject) => {
    const client = new net.Socket();
    let dataBuffer = '';

    client.connect(BACKEND_PORT, BACKEND_HOST, () => {
      console.log(`Connected to backend at ${BACKEND_HOST}:${BACKEND_PORT}`);
      const msg = message.endsWith('\r\n\r\n') ? message : message + '\r\n\r\n';
      client.write(msg);
    });

    // Backend sends response, we resolve here
    client.on('data', (data) => {
      dataBuffer += data.toString();
    });

    // Backend closes connection, we resolve only once
    client.on('close', () => {
      if (dataBuffer.length > 0) {
        resolve(dataBuffer);
      } else {
        reject(new Error('Backend closed connection without data'));
      }
    });

    client.on('error', (err) => reject(err));
  });
}

// HTTP endpoint (signup, etc.)
// When arrive a HTTP request at /api/send executes this call back function
// In req we have all information sent from frontend
app.post('/api/send', async (req, res) => {
  try {
    const message = req.body.message;
    if (!message) return res.status(400).json({ error: 'Missing message field' });

    console.log(`Forwarding one-shot request to backend: ${message}`);
    const response = await sendToBackend(message);
    console.log(`Backend responded: ${response}`);

    //We send HTTP backend response to client 
    res.json({ backendResponse: response.trim() });
  } catch (err) {
    console.error('Bridge error:', err);
    res.status(500).json({ error: 'Failed to communicate with backend' });
  }
});

// Start the bridge HTTP server
app.listen(HTTP_PORT, () => {
  console.log(`HTTP Bridge listening on port ${HTTP_PORT} (non-persitent session)`);
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

  //Client represents the TCP connection (with backend)
  //ws represents the WebSocket connection (with frontend)
  client.connect(BACKEND_PORT, BACKEND_HOST, () => {
    console.log(`Connected persistent TCP → ${BACKEND_HOST}:${BACKEND_PORT}`);

    //Print table connection 
    console.log(`\n Numero di connessioni websocket aperte: ${userConnections.size}`);
    console.log("\n=== CONNESSIONI ATTUALI ===\n");

    const connectionsInfo = [];

    for (const [ws, client] of userConnections) {
      connectionsInfo.push({
        WebSocketState: ws.readyState,
        TCP: `${client.remoteAddress}:${client.remotePort}`
      });
    }

    console.table(connectionsInfo);
  });

  // --- FRONTEND → BACKEND ------------------------------------------------
  ws.on('message', (msg) => {
    const message = msg.toString();
    console.log(`[Frontend → Backend]: ${message}`);
    const withDelim = message.endsWith('\r\n\r\n') ? message : message + '\r\n\r\n';
    client.write(withDelim);
  });

  // --- BACKEND → FRONTEND ------------------------------------------------
  client.on('data', (data) => {
    const str = data.toString();
    console.log(`[Backend → Frontend]: ${str}`);
    ws.send(JSON.stringify({ backendResponse: str.trim() }));
  });

  // --- CONNECTION MANAGEMENT ---------------------------------------------
  ws.on('close', () => {
    console.log('Frontend disconnected — closing backend TCP');
    client.end();
    userConnections.delete(ws);

  });

  client.on('close', () => {
    console.log('Backend closed connection — closing WebSocket');
    //After the TCP connection with backend is closed, we olso close WebSocket connection with frontend
    if (ws.readyState === ws.OPEN) ws.close();
    userConnections.delete(ws);
  });

  client.on('error', (err) => {
    console.error('TCP connection error:', err.message);
    if (ws.readyState === ws.OPEN)
      ws.send(JSON.stringify({ error: 'TCP connection error: ' + err.message }));
  });
});

console.log(`WebSocket Bridge listening on port ${WS_PORT} (persistent sessions)`);