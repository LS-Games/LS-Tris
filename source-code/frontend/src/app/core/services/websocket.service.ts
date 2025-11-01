import { inject, Injectable } from '@angular/core';
import { ConfigService } from './config.service';

/**
 * WebsocketService
 * 
 * This service manages a WebSocket connection between the Angular frontend
 * and the Node.js bridge. It allows the app to open a connection, send messages,
 * receive data in real time, and close the connection when needed.
 */
@Injectable({
  providedIn: 'root' // The service is a singleton and available globally in the app
})
export class WebsocketService {
  // Inject environment variables
  private readonly config = inject(ConfigService);

  // Holds the WebSocket instance. The "?" means it can be undefined until initialized.
  private socket?: WebSocket;

  /**
   * Opens a WebSocket connection to the specified URL.
   * If no URL is provided, it defaults to ws://localhost:3002.
   */
  private readonly address = this.config.get('BRIDGE_ADDRESS');
  private readonly wsPort = this.config.get('BRIDGE_WS_PORT');
  connect(url: string = `ws://${this.address}:${this.wsPort}`): Promise<void> {
  return new Promise((resolve, reject) => {
    this.socket = new WebSocket(url);

    this.socket.onopen = () => {
      console.log('Connected to WebSocket:', url);
      resolve();
    };

    this.socket.onerror = (error) => {
      console.error('WebSocket error:', error);
      reject(error);
    };
  });
}

  /** Public method: register a message callback */
  onMessage(callback: (event: MessageEvent) => void): void {
    if (!this.socket) {
      console.warn('WebSocket not connected yet.');
      return;
    }
    this.socket.onmessage = callback;
  }

  /**
   * Sends data to the server through the WebSocket connection.
   * If the connection is not yet open, it logs a warning instead of throwing an error.
   */
  send(data: any): void {
    if (this.socket && this.socket.readyState === WebSocket.OPEN) {
      // Convert the object into a JSON string before sending
      this.socket.send(JSON.stringify(data));
    } else {
      console.warn('WebSocket not connected yet.');
    }
  }

  /**
   * Closes the WebSocket connection if it exists.
   * Useful for cleanup when a component is destroyed or the user logs out.
   */
  close(): void {
    this.socket?.close(); // The "?" prevents errors if socket is undefined
  }
}
