import { Component, inject, afterNextRender, DestroyRef } from '@angular/core';
import { Dialog, DialogRef } from '@angular/cdk/dialog';
import { WebsocketService } from '../../core/services/websocket.service';
import { NotificationService } from '../../core/services/notification';
import { GameCard } from '../lobby/components/game-card/game-card';
import { RequestPage } from '../request-page/request-page';
import { takeUntilDestroyed } from '@angular/core/rxjs-interop';
import { GameService } from '../../core/services/game.service';

@Component({
  selector: 'app-lobby',
  standalone: true,
  imports: [GameCard],
  templateUrl: './lobby.html',
  styleUrl: './lobby.scss',
})

export class Lobby {

  private readonly _dialog = inject(Dialog)
  private readonly _dialogRef = inject(DialogRef<Lobby>);
  private readonly _ws = inject(WebsocketService);
  private readonly _notification = inject(NotificationService);
  private readonly _destroyRef = inject(DestroyRef); // replaces ngOnDestroy
  private readonly _game = inject(GameService);

  loading = true;
  games: any[] = [];

  constructor() {

    /**
     * afterNextRender() is executed after the component’s initial render.
     * Equivalent to ngOnInit(), but works in a functional and signal-friendly way.
     */

    afterNextRender(() => {
      this._game
        .getAllGame()
          .pipe(takeUntilDestroyed(this._destroyRef))
          .subscribe({

            /**
             * This next is different to Subject.next(), in this case next: is useful to 
             * perform a specific action when a event occurs, while in the first case is used to 
             * send a new value to the listeners
             */

            next : (backend) => {
              if(backend.games && backend.count != 0) {
                this.games = backend.games;
                this.loading = false;

                if(backend.count == 0) {
                  this._notification.show('info', 'There are no games available right now.')
                }

              } else {
                  this._notification.show('error', backend.error_message || 'Failed to fetch games.');
                  this.loading = false;
              }
            },

            error: (err) => {
              console.error('WebSocket error:', err);
              this.loading = false;
              this._notification.show('error', 'An unexpected error occurred.');
            }
          })
    });
  }

  /** Closes the dialog */
  close() {
    this._dialogRef.close();
  }

  //Logic to open game request page when create game button is clicked"
  openRequestPage() {
    this._dialog.open(RequestPage, {
      disableClose: true
    })

    //We close the Lobby section 
    this.close();
  }
}
