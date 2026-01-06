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

interface BackendPlayerInfoResponse {
  action: 'player_get_public_info';
  status: 'success' | 'error';
  players?: PlayerInfo[];
  error_message?: string;
}

export interface PlayerInfo {
  id_player: number;
  nickname: string;
  email: string;
  current_streak: number;
  max_streak: number;
  registration_date: string;
}

@Injectable({ providedIn: 'root' })
export class AuthService {

  private readonly _http = inject(HttpService);
  private readonly _ws = inject(WebsocketService);

  // --- AUTH STATE ---
  private readonly _playerId = signal<number | null>(null);
  readonly currentUser = signal<PlayerInfo | null>(null);

  readonly isLoggedIn = computed(() => this._playerId() !== null);
  get id() { return this._playerId(); }

  private readonly loginSuccess$ = new Subject<number>();
  private readonly loginError$ = new Subject<string>();

  constructor() {

    // --- SESSION RESTORE (ID ONLY) ---
    const savedId = localStorage.getItem('player_id');
    if (savedId) {
      this._playerId.set(Number(savedId));
    }

    /**
     * LocalStorage sync
     */
    effect(() => {
      const user = this.currentUser();
      const id = this._playerId();

      if (id !== null) {
        localStorage.setItem('player_id', String(id));
        localStorage.setItem('token', 'demo');

        if (user) {
          localStorage.setItem('player_nickname', user.nickname);
        }
      } else {
        localStorage.removeItem('player_id');
        localStorage.removeItem('player_nickname');
        localStorage.removeItem('token');
        this.currentUser.set(null);
      }
    });

    // --- LOGIN RESPONSE ---
    this._ws.onAction<BackendSigninResponse>('player_signin')
      .subscribe((backend) => {
        if (backend.status === 'success' && backend.id) {
          this._playerId.set(backend.id);
          this.loginSuccess$.next(backend.id);
        } else {
          this.loginError$.next(backend.error_message || 'Login failed');
        }
      });

    // --- PLAYER INFO RESPONSE ---
    this._ws.onAction<BackendPlayerInfoResponse>('player_get_public_info')
      .subscribe((msg) => {
        if (msg.status === 'success' && msg.players && msg.players.length > 0) {
          this.currentUser.set(msg.players[0]);
        }
      });
  }

  /**
   * Ensures that currentUser is loaded when a session exists.
   * Call this when entering protected pages (e.g. Profile).
   */
  ensureUserLoaded(): void {
    if (!this.isLoggedIn()) return;
    if (this.currentUser() !== null) return;

    const nickname = localStorage.getItem('player_nickname');
    if (!nickname) {
      // Inconsistent session → force logout
      this.logout();
      return;
    }

    this._ws.connect().then(() => {
      this.fetchPlayerInfo(nickname);
    });
  }

  // --- Helper Methods ---

  fetchPlayerInfo(nickname: string) {
    this._ws.send({
      action: 'player_get_public_info',
      nickname
    });
  }

  // --- PUBLIC API ---

  signup(nickname: string, email: string, password: string) {
    return this._http.send({ action: 'player_signup', nickname, email, password });
  }

  signin(nickname: string, password: string) {
    this._ws.reset();

    this._ws.connect().then(() => {
      this._ws.send({ action: 'player_signin', nickname, password });
      this.fetchPlayerInfo(nickname);
    });
  }

  logout() {
    this._playerId.set(null);
    this._ws.reset();
  }

  onLoginSuccess() { return this.loginSuccess$.asObservable(); }
  onLoginError() { return this.loginError$.asObservable(); }
}
