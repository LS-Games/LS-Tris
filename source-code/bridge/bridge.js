// Import the required modules
import express from 'express';   // Express: lightweight web server for Node.js
import net from 'net';           // 'net' is Node's built-in module for TCP sockets

// Create an Express application instance
const app = express();

// Enable CORS for all routes
app.use((req, res, next) => {
  res.header("Access-Control-Allow-Origin", "*"); // allow all origins
  res.header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
  res.header("Access-Control-Allow-Headers", "Content-Type, Authorization");
  if (req.method === "OPTIONS") {
    return res.sendStatus(200);
  }
  next();
});

// This middleware automatically parses incoming JSON requests
app.use(express.json());

// Environment variables (set in docker-compose or locally)

// *!*!*!*!*! REMEMBER TO CHANGE localhost WITH backend that is the service name in docker compose *!*!*!*!*!
// *!*!*!*!*! REMEMBER TO CHANGE bakcend port WITH the true backend port in backend dockercompose *!*!*!*!*!

const BACKEND_HOST = process.env.TCP_HOST || 'localhost'; // default host = container named 'backend'
const BACKEND_PORT = parseInt(process.env.TCP_PORT || '5050'); // default port = 8080

// Helper function to communicate with the C backend via TCP
function sendToBackend(message) {
  return new Promise((resolve, reject) => {
    // Create a new TCP socket 
    const client = new net.Socket();

    // We'll store the backend's response here
    let dataBuffer = '';

    // When connection is successfully established
    client.connect(BACKEND_PORT, BACKEND_HOST, () => {
      console.log(`Connected to backend at ${BACKEND_HOST}:${BACKEND_PORT}`);
      // Send the message to the backend as raw text
      //We use '/n' at end because we want know if the json is complete
      client.write(message);

      //This client.end() is very important because if we don't do this after send a message
      //The recv() function on backend may remain waiting for additional bytes 
      //Whereas this way we signal him to stop waiting
      client.end();
    });

    //client.on are a events listener
    // When data is received from the backend
    client.on('data', (data) => {
      // Convert the received Buffer into a string and append it to the buffer
      dataBuffer += data.toString();
      // The C server closes the connection after replying, so we can safely close it here
      client.destroy();
    });

    // When the connection is closed (either by us or the server)
    client.on('close', () => {
      
      // Resolve the Promise with the full response string
      resolve(dataBuffer);
    });

    // If an error occurs during the connection
    client.on('error', (err) => {
      console.error('TCP connection error:', err.message);
      reject(err);
    });
  });
}

// Express endpoint: receives messages from the frontend
app.post('/api/send', async (req, res) => {
  try {
    // Extract the 'message' field from the JSON body
    const message = req.body.message;

    // If the client didn't send any message, return HTTP 400
    if (!message) {
      return res.status(400).json({ error: 'Missing message field' });
    }

    console.log(`Forwarding to backend: ${message}`);

    // Send message to backend through the TCP helper function
    const response = await sendToBackend(message);

    //Capture and print the backend response
    console.log(`Received from backend: ${response}`);

    // Return the backend's response to the frontend in JSON format
    res.json({ backendResponse: response.trim() });

  } catch (err) {
    // If something goes wrong (timeout, network, etc.)
    console.error('Bridge error:', err);
    res.status(500).json({ error: 'Failed to communicate with backend' });
  }
});

// Start the bridge HTTP server
app.listen(3001, () => {
  console.log(`Bridge listening on port 3001 → forwarding to ${BACKEND_HOST}:${BACKEND_PORT}`);
});
