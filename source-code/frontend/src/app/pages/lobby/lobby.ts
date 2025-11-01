import { Component, inject, afterNextRender, DestroyRef, NgZone } from '@angular/core';
import { DialogRef } from '@angular/cdk/dialog';
import { WebsocketService } from '../../core/services/websocket.service';
import { NotificationService } from '../../core/services/notification';
import { GameCard } from '../lobby/components/game-card/game-card';

@Component({
  selector: 'app-lobby',
  standalone: true,
  imports: [GameCard],
  templateUrl: './lobby.html',
  styleUrl: './lobby.scss',
})
export class Lobby {

  private readonly _dialogRef = inject(DialogRef<Lobby>);
  private readonly _ws = inject(WebsocketService);
  private readonly _notification = inject(NotificationService);
  private readonly _destroyRef = inject(DestroyRef); // replaces ngOnDestroy
  private readonly _zone = inject(NgZone);

  loading = true;

  games: any[] = [];

  constructor() {

    /**
     * afterNextRender() is executed after the component’s initial render.
     * Equivalent to ngOnInit(), but works in a functional and signal-friendly way.
     */


    afterNextRender(() => {

      // Step 1: send the request to the backend
      const payload = { action: 'games_get_public_info', status: 'all' };
      this._ws.send(payload);

      // Step 2: define the listener
      const messageListener = (event: MessageEvent) => {
        try {

          console.log("Sono nel listener")
          const data = JSON.parse(event.data);
          const backend = JSON.parse(data.backendResponse);

          this._zone.run(() => {
      
            if(backend.games) {
              this.games = backend.games || [];
              this.loading = false;
            }

            if (this.games.length === 0) {
              this._notification.show('info', 'There are no games available right now.');
            }

          });

        } catch (err) {
          console.error('Failed to parse backend response:', err);
          this.loading = false;
        }
      };

      // Step 3: start listening
      this._ws.onMessage(messageListener);

      /**
       * When the component is destroyed, Angular automatically runs this callback.
       * This replaces ngOnDestroy() and ensures perfect cleanup.
       * 
       * Reset the listener, we can do it overwriting it with a function that does nothing 
       * if we hadn't done this, it would have executed the function every time data arrived.  
       */

      this._destroyRef.onDestroy(() => {
        this._ws.onMessage(() => {}); // reset the listener safely
      });
    });
  }

  /** Closes the dialog */
  close() {
    this._dialogRef.close();
  }
}
