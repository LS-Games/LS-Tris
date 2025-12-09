import { afterNextRender, Component, inject, DestroyRef, effect } from '@angular/core';
import { DialogRef } from '@angular/cdk/dialog';
import { GameService } from '../../core/services/game.service';
import { WebsocketService } from '../../core/services/websocket.service';
import { takeUntilDestroyed } from '@angular/core/rxjs-interop';
import { RequestsService } from '../../core/services/requests.service';
import { RequestCard } from './components/request-card/request-card';
import { Router } from '@angular/router';
import { RoundService } from '../../core/services/round.service';

@Component({
  selector: 'app-request-page',
  standalone: true,
  imports: [RequestCard],
  templateUrl: './request-page.html',
  styleUrl: './request-page.scss'
})
export class RequestPage {

  private readonly _dialogRef = inject(DialogRef<RequestPage>);
  private readonly _destroyRef = inject(DestroyRef);
  private readonly _game = inject(GameService);
  private readonly _ws = inject(WebsocketService);
  private readonly _rqst = inject(RequestsService);
  private readonly _round = inject(RoundService);
  private readonly _router = inject(Router);

  loading = true;
  requests = this._rqst.requestsSignal;

  id_game?: number;
  error_message: string | null = null;

  constructor() {

    effect(() => {
      const gameId = this._round.gameId();
      const roundId = this._round.roundId();

      if (gameId !== null && roundId !== null) {
        this._dialogRef.close();
        this._router.navigate(['/round', gameId, roundId]);
      }
    });

    afterNextRender(() => {

      //When a create game button in Lobby is clicked we open a request-page and we create a game
      this._game.createGame();

      this._ws.onAction<any>('game_start')
        .pipe(takeUntilDestroyed(this._destroyRef))
        .subscribe(msg => {

          if (msg.status === 'success' && msg.id) {
            this.id_game = msg.id;
            this.loading = false;
            console.log("Game created successfully:", msg.id);
          } else {
            this.error_message = msg.error_message || 'Unknown error';
            this.loading = false;
            console.error("Game creation error:", this.error_message);
            this.close();
          }
        });
    });
  }

  close() {
    if (this.id_game === undefined) {
      this._dialogRef.close();
      return;
    }

    this._rqst.rejectAllParticipationRequests();
    this._rqst.clearRequests();
    this._game.deleteGame(this.id_game);
    this._dialogRef.close();
  }
}
