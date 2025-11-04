import { inject, Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { ConfigService } from './config.service';
import { Observable } from 'rxjs';

/**
 * Handles one-shot HTTP requests (non-persistent)
 * toward the bridge, such as signup or password recovery.
 */
@Injectable({ providedIn: 'root' })
export class HttpService {
  private readonly _http = inject(HttpClient);
  private readonly _config = inject(ConfigService);

  private readonly baseUrl = `http://${this._config.get('BRIDGE_ADDRESS')}:${this._config.get('BRIDGE_HTTP_PORT')}`;

  /**
   * Generic send function, we return a Observable so we can leave the 
   * management to the caller 
   */
  send(payload: any): Observable<any> {
    return this._http.post(`${this.baseUrl}/api/send`, { message: JSON.stringify(payload) });
  }
}
