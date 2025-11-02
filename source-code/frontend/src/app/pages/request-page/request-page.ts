import { afterNextRender, Component, inject, DestroyRef, NgZone } from '@angular/core';
import { DialogRef } from '@angular/cdk/dialog';
import { AuthService } from '../../core/services/auth.service';
import { WebsocketService } from '../../core/services/websocket.service';

@Component({
  selector: 'app-request-page',
  standalone: true,
  imports: [],
  templateUrl: './request-page.html',
  styleUrl: './request-page.scss'
})
export class RequestPage {

  private readonly _dialogRef = inject(DialogRef<RequestPage>);
  private readonly _authService = inject(AuthService);
  private readonly _ws = inject(WebsocketService);
  private readonly _zone = inject(NgZone);
  private readonly _destroyRef = inject(DestroyRef);

  loading = true;
  request :any[] = [];
  id_game = -1;
  

  constructor() {

    afterNextRender(() => {

      const payload = { action: 'game_start', id_creator: `${this._authService.id}`};
      this._ws.send(payload);

      const messageListener = (event: MessageEvent) => {
        try {
          const data = JSON.parse(event.data);
          const backend = JSON. parse(data.backendResponse);

          console.log(backend);

          this._zone.run(() => {
            
            if(backend.status === 'success') {
              this.id_game = backend.id;

            } else if(backend.status === 'error') {
              this.id_game = -1;
            }

          })

        } catch (err) {
            console.error('Failed to parse backend response:', err);
        }
      };

      this._ws.onMessage(messageListener);

      this._destroyRef.onDestroy(() => {
        this._ws.onMessage(() => {}); // reset the listener safely
      });
    });
  }

  /**
   * We have to do a method that deletes a game after than the creator has clsoed the 
   * waiting section whitout starting a new game.
   */
  // closeGame() {} 

  close() {
    this._dialogRef.close();

  }
}
