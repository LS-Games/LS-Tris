import { Component, inject, effect } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BoardComponent } from '../round-page/components/board/board';
import { RoundService } from '../../core/services/round.service';
import { GameService } from '../../core/services/game.service';
import { Router } from '@angular/router';
import { CanComponentDeactivate } from '../../core/guards/can-component-deactivate';
import { Dialog } from '@angular/cdk/dialog';
import { RequestPage } from '../request-page/request-page';
import { RequestsService } from '../../core/services/requests.service';
@Component({
  selector: 'app-round-page',
  standalone: true,
  imports: [CommonModule, BoardComponent],
  templateUrl: './round-page.html',
  styleUrl: './round-page.scss'
})
export class RoundPage implements CanComponentDeactivate {

  private readonly _round = inject(RoundService);
  private readonly _router = inject(Router);
  private readonly _game = inject(GameService);
  private readonly _dialog = inject(Dialog);
  private readonly _rqst = inject(RequestsService);

  private allowInternalNavigation = false;
  
  player1Nickname = this._round.player1Nickname;
  player2Nickname = this._round.player2Nickname;

  board = this._round.boardSignal;
  currentPlayer = this._round.currentPlayerTurnSignal;
  mySimbol = this._round.mySymbolSignal;
  winner = this._round.winnerSignal;
  roundEnded = this._round.roundEndedSignal;
  rematchPending = this._round.rematchPendingSignal;
  winnerByForfeit = this._round.winnerByForfeitSignal;

  constructor() {

    effect(() => {
      const winner = this.winner();
      if (winner) {
        console.log("Round finito:", winner);
      }
    });

    // Keep URL in sync with the current round (rematch included)
    effect(() => {
      const gameId = this._round.gameId();
      const roundId = this._round.roundId();

      if (gameId !== null && roundId !== null) {
        this._router.navigate(['/round', gameId, roundId], { replaceUrl: true });
      }
    });
  }

  handleMove(index:number) {

    if(this.currentPlayer() !== this.mySimbol()) {
      console.log("Non è il tuo turno");
      return;
    }

    if(this.board()[index] !== null) return;

    this._round.makeMove(index);
  }

  toHomePage() {

    this.allowInternalNavigation = true;
    this._round.resetAll();
    this._router.navigate(['']);
  }

  toHomePageAfterDraw() {
    this.allowInternalNavigation = true;

    if(!this.winnerByForfeit()) {
      console.log(this.winnerByForfeit());
      this._game.forfeitGame();
    }

    this._round.resetAll();
    this._router.navigate(['']);
  }

  newGameAfterWin() {
    this.openRequestPage();
  }

  gameEndAfterWin() {
    this.allowInternalNavigation = true;
    this._game.endGame();
    this.toHomePage();
  }

  rematch() {
    this.allowInternalNavigation = true;
    this._game.rematchGame(); 
  }

  closePending() {
    this._game.refuseRematchGame();
    this._round.endPending();
  }

  canDeactivate(): boolean {

    if (this._round.isNavigationAllowedSignal()) {
      return true;
    }

    if(this.allowInternalNavigation) {
      this.allowInternalNavigation = false;
      return true;
    }

    if (this.roundEnded()) {
      return true;
    }

    const confirmLeave = confirm('If you leave now, the match will be forfeited. Do you want to continue?');

    if (confirmLeave) {

      if(!this.winnerByForfeit()) { 
        this._game.forfeitGame();
      }
    }

    return confirmLeave;
  }

  openRequestPage() {
    this._dialog.open(RequestPage, {
      disableClose: true
    })
  }

}
