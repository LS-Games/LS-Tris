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

  loading = true;
  requests = this._rqst.requestsSignal;

  id_game?: number;
  error_message: string | null = null;

  constructor() {

    const gameId = this._round.gameId();

    if(gameId !== null) {
      this.id_game = gameId;
      this.loading = false;
    }

    afterNextRender(() => {

      this._ws.onAction<any>('game_start')
        .pipe(takeUntilDestroyed(this._destroyRef))
        .subscribe(msg => {

          console.log(msg);

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

    /**
     * If there is a round id (so it was played) or the round is in a "finished" status
     * then don't delete it    
    */

    const hasHistory = this._round.roundId() !== null || this._round.roundEndedSignal();

    if(!hasHistory) {
      this._rqst.rejectAllParticipationRequests();
      this._rqst.clearRequests();
      this._game.deleteGame(this.id_game);
    
      //There is a history, don't delete it

    } else {
      this._rqst.rejectAllParticipationRequests();
      this._rqst.clearRequests();
    }
    this._dialogRef.close();
  }
}
