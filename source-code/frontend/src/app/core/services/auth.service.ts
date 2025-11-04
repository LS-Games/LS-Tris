import { Injectable, signal, computed, effect, inject } from '@angular/core';
import { HttpService } from './http.service';
import { WebsocketService } from './websocket.service';
import { Subject } from 'rxjs';

interface BackendSigninResponse {
  action: 'player_signin';
  status: 'success' | 'error';
  id?: number;
  error_message?: string;
}

@Injectable({ providedIn: 'root' })
export class AuthService {

  private readonly _http = inject(HttpService);
  private readonly _ws = inject(WebsocketService);

  private readonly _isLoggedIn = signal(false);
  readonly isLoggedIn = computed(() => this._isLoggedIn());
  private readonly _playerId = signal<number | null>(null);

  /**
   * We use Subject in this case becase we can notify our listeners which need
   * to know if the login was succesfull 
   */

  private readonly loginSuccess$ = new Subject<number>();
  private readonly loginError$ = new Subject<string>();

  constructor() {

    const saved = localStorage.getItem('player_id');
    if (saved) this._playerId.set(Number(saved));

    effect(() => {
      if (this.isLoggedIn()) localStorage.setItem('token', 'demo');
      else localStorage.removeItem('token');
    });
  }

  // --- SIGNUP (HTTP, no persistence) --- 
  signup(nickname: string, email: string, password: string) {
    const payload = { action: 'player_signup', nickname, email, password };
    return this._http.send(payload);
  }

  // --- SIGNIN (WebSocket, persistence) --- 
signin(nickname: string, password: string) {
  const payload = { action: 'player_signin', nickname, password };

  this._ws.connect().then(() => {
    this._ws.send(payload);

    this._ws.onAction<BackendSigninResponse>('player_signin')
      .subscribe((backend) => {
        console.log('Received backend response for login:', backend);

        if (backend.status === 'success' && backend.id) {
          this._playerId.set(backend.id);
          this._isLoggedIn.set(true);
          this.loginSuccess$.next(backend.id);
        } else {
          this.loginError$.next(backend.error_message || 'Login failed');
        }
      });
  });
}


  onLoginSuccess() {
    return this.loginSuccess$.asObservable();
  }

  onLoginError() {
    return this.loginError$.asObservable();
  }

  logout() {
    this._isLoggedIn.set(false);
    this._playerId.set(null);
    localStorage.removeItem('player_id');
    localStorage.removeItem('token');
    this._ws.close();
  }

  get id() {
    return this._playerId();
  }

  set id(value: number | null) {
    this._playerId.set(value);
    if (value !== null) localStorage.setItem('player_id', String(value));
    else localStorage.removeItem('player_id');
  }
}
