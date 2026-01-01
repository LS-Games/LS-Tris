import { Injectable, signal, computed, effect, inject } from '@angular/core';
import { HttpService } from './http.service';
import { WebsocketService } from './websocket.service';
import { Subject } from 'rxjs';

// --- Interfaces ---

interface BackendSigninResponse {
  action: 'player_signin';
  status: 'success' | 'error';
  id?: number;
  error_message?: string;
}

/**
 * Matches the JSON structure returned by the C server's 
 * 'serialize_players_to_json' function.
 */
interface BackendPlayerInfoResponse {
  action: 'player_get_public_info';
  status: 'success' | 'error';
  players?: PlayerInfo[]; // The server returns an array
  error_message?: string;
}

export interface PlayerInfo {
  id_player: number;
  nickname: string;
  email: string; // Ensure the server sends this in public info, otherwise optional
  current_streak: number;
  max_streak: number;
  registration_date: string;
}

@Injectable({ providedIn: 'root' })
export class AuthService {

  private readonly _http = inject(HttpService);
  private readonly _ws = inject(WebsocketService);

  // 1. Single source of truth: the Player ID
  private readonly _playerId = signal<number | null>(null);

  // Signal to hold the full player details (nickname, stats, etc.)
  readonly currentUser = signal<PlayerInfo | null>(null);

  /**
   * 2. isLoggedIn is automatically COMPUTED based on the ID.
   * If _playerId has a value, the user is logged in.
   * This prevents state inconsistencies.
   */
  readonly isLoggedIn = computed(() => this._playerId() !== null);
  
  // Public getter to access the signal value directly
  get id() { return this._playerId(); }

  /**
   * We use Subjects here to notify listeners (components) about
   * the outcome of the login attempt.
   */
  private readonly loginSuccess$ = new Subject<number>();
  private readonly loginError$ = new Subject<string>();

  constructor() {
    
    // 3. Initial load: Check if there is a saved session
    const savedId = localStorage.getItem('player_id');
    const savedNickname = localStorage.getItem('player_nickname');

    // If we have both ID and Nickname, restore session and fetch latest stats
    if (savedId && savedNickname) {
      this._playerId.set(Number(savedId));
      this.reconnectAndFetch(savedNickname);
    }

    /**
     * 4. EFFECT: Automatically manages LocalStorage persistence.
     * This effect runs every time the _playerId or currentUser signal changes.
     */
    effect(() => {
      const user = this.currentUser();
      const id = this._playerId();

      if (id !== null) {
        localStorage.setItem('player_id', String(id));
        localStorage.setItem('token', 'demo'); // Simulated token management
        
        // We also persist the nickname to allow data fetching on page reload
        if (user) {
          localStorage.setItem('player_nickname', user.nickname);
        }
      } else {
        localStorage.removeItem('player_id');
        localStorage.removeItem('player_nickname');
        localStorage.removeItem('token');
        
        // Clear user data on logout
        this.currentUser.set(null);
      }
    });

    // --- LISTENER 1: Handle WebSocket login response ---
    this._ws.onAction<BackendSigninResponse>('player_signin')
      .subscribe((backend) => {
        console.log('Received backend response for login:', backend);

        if (backend.status === 'success' && backend.id) {
          // Update the ID signal (the EFFECT will handle storage persistence)
          this._playerId.set(backend.id);

          // Notify components that login succeeded
          this.loginSuccess$.next(backend.id);
        } else {
          this.loginError$.next(backend.error_message || 'Login failed');
        }
      });

    // --- LISTENER 2: Handle Player Info response ---
    this._ws.onAction<BackendPlayerInfoResponse>('player_get_public_info')
      .subscribe((msg) => {
        console.log('Received player info:', msg);

        if (msg.status === 'success' && msg.players && msg.players.length > 0) {
          // The C server returns an array, we take the first element
          this.currentUser.set(msg.players[0]);
        }
      });
  }

  // --- Helper Methods ---

  /**
   * Sends a request to the server to get player details.
   * Based on the C backend, this action requires the 'nickname'.
   */
  fetchPlayerInfo(nickname: string) {
    const payload = {
      action: 'player_get_public_info',
      nickname: nickname
    };
    this._ws.send(payload);
  }

  /**
   * Used on page reload: ensures WebSocket connection is open
   * before requesting player info.
   */
  private reconnectAndFetch(nickname: string) {
    this._ws.connect().then(() => {
      this.fetchPlayerInfo(nickname);
    });
  }

  // --- PUBLIC API ---

  // --- SIGNUP (HTTP, no persistence needed immediately) ---
  signup(nickname: string, email: string, password: string) {
    const payload = { action: 'player_signup', nickname, email, password };
    return this._http.send(payload);
  }

  // --- SIGNIN (WebSocket) ---
  signin(nickname: string, password: string) {
    const payload = { action: 'player_signin', nickname, password };
    
    // Ensure clean state before connecting
    this._ws.reset();

    this._ws.connect().then(() => {
      // 1. Send Login Request
      this._ws.send(payload);

      // 2. Immediately request player info (Optimistic fetch)
      // We do this here because we have the 'nickname' variable available.
      this.fetchPlayerInfo(nickname);
    });
  }

  logout() {
    // Setting this to null triggers the EFFECT to clean up localStorage
    this._playerId.set(null); 
    this._ws.reset();
  }

  // Expose Observables for components to subscribe to login events
  onLoginSuccess() { return this.loginSuccess$.asObservable(); }
  onLoginError() { return this.loginError$.asObservable(); }
}