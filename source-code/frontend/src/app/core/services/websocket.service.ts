import { inject, Injectable, NgZone } from '@angular/core';
import { ConfigService } from './config.service';
import { Subject, Observable } from 'rxjs';
import { filter, map } from 'rxjs/operators';

/**
 * Interface describing the general structure of messages
 * exchanged between the frontend and backend via WebSocket.
 * Each message must contain an `action` field,
 * but may include additional dynamic fields.
 */
interface BackendMessage {
  action: string;
  [key: string]: any; // dynamic fields (id, games, error_message, etc.)
}

@Injectable({ providedIn: 'root' })
export class WebsocketService {
  // Inject app configuration (used to build the WebSocket URL)
  private readonly config = inject(ConfigService);

  // NgZone is used to bring asynchronous events (from WebSocket)
  // back into Angular's change detection system
  private readonly _zone = inject(NgZone);

  // Holds the WebSocket instance (undefined until connection is established)
  private socket?: WebSocket;

  // Shared reactive stream that emits every message received from the backend
  // This Subject acts as a bridge between the native WebSocket events
  // and the rest of the Angular app (components, services, etc.)
  private readonly messages$ = new Subject<BackendMessage>();

  // WebSocket connection parameters
  private readonly address = this.config.get('BRIDGE_ADDRESS');
  private readonly wsPort = this.config.get('BRIDGE_WS_PORT');

  /**
   * Establishes a single WebSocket connection to the backend.
   * This should typically be called once when the app starts.
   * 
   * @param url - Optional custom WebSocket endpoint.
   * @returns A Promise that resolves when the connection is successfully opened.
   */
  connect(url: string = `ws://${this.address}:${this.wsPort}`): Promise<void> {
    return new Promise((resolve, reject) => {
      this.socket = new WebSocket(url);

      // Triggered once the connection is established
      this.socket.onopen = () => {
        console.log('Connected to WebSocket:', url);
        resolve();
      };

      // Triggered when an error occurs (e.g., connection refused)
      this.socket.onerror = (error) => {
        console.error('WebSocket error:', error);
        reject(error);
      };

      // Single event listener for all incoming WebSocket messages
      // Each message is parsed and emitted through the reactive Subject
      this.socket.onmessage = (event: MessageEvent) => {
        this._zone.run(() => {
          try {
            const wrapper = JSON.parse(event.data);

            let payload: BackendMessage | null = null;

            // Standard path: the bridge sends { backendResponse: "<json string>" }
            if (typeof wrapper.backendResponse === 'string') {
              try {
                payload = JSON.parse(wrapper.backendResponse) as BackendMessage;
              } catch (innerErr) {
                console.error(
                  'Failed to parse backendResponse JSON:',
                  innerErr,
                  'backendResponse:',
                  wrapper.backendResponse
                );
              }
            }
            // Fallback: sometimes the bridge may send a direct object with action
            else if (wrapper && typeof wrapper.action === 'string') {
              payload = wrapper as BackendMessage;
            }
            // Error-only messages from the bridge (no action, no backendResponse)
            else if (wrapper && wrapper.error) {
              payload = {
                action: 'connection_error',
                ...wrapper,
              } as BackendMessage;
            }

            if (payload) {
              this.messages$.next(payload);
            } else {
              console.warn(
                'Received message without recognizable payload:',
                wrapper
              );
            }
          } catch (err) {
            console.error(
              'Invalid JSON received from server:',
              err,
              'Raw data:',
              event.data
            );
          }
        });
      };

      // Triggered when the connection is closed by the client or server
      this.socket.onclose = () => {
        console.warn('WebSocket connection closed');
      };
    });
  }

  /**
   * Returns a shared Observable that emits all messages received from the backend.
   * 
   * Components and services can subscribe to this Observable
   * to react to every incoming message in real time.
   */
  onMessage(): Observable<BackendMessage> {
    return this.messages$.asObservable();
  }

  /**
   * Utility method that filters the message stream
   * to only include messages with a specific `action` type.
   * 
   * @param action - The backend action to listen for (e.g., "game_start").
   * @returns An Observable emitting only messages of the given action type.
   */
  onAction<T = any>(action: string): Observable<T> {
    return this.messages$.pipe(
      filter(msg => msg.action === action),
      map(msg => msg as unknown as T)
    );
  }

  /**
   * Sends a message (object) to the backend through the WebSocket.
   * Automatically stringifies the payload to JSON before sending.
   * Logs a warning if the connection is not yet open.
   * 
   * @param data - Any serializable object to send to the backend.
   */
  send(data: any): void {
    if (this.socket && this.socket.readyState === WebSocket.OPEN) {
      this.socket.send(JSON.stringify(data));
    } else {
      console.warn('WebSocket not connected yet.');
    }
  }

  /**
   * Gracefully closes the WebSocket connection.
   * Also completes the Subject to stop all active subscriptions cleanly.
   * Should be called when the user logs out or the app shuts down.
   */
  close(): void {
    if (this.socket) {
      this.socket.close();
      this.messages$.complete();
      console.log('WebSocket connection closed.');
    }
  }
}
