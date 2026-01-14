import { Component, inject, effect, signal, computed } from '@angular/core';
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
  styleUrls: ['./round-page.scss']
})

export class RoundPage implements CanComponentDeactivate {

  private readonly _round = inject(RoundService);
  private readonly _router = inject(Router);
  private readonly _game = inject(GameService);
  private readonly _dialog = inject(Dialog);
  private readonly _rqst = inject(RequestsService);

  private debugEnabled = true;

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

  startTimeMs = signal<number | null>(null);
  elapsedSeconds = signal<number>(0);
  timerId: any = null;

  roundActive = computed(() =>
    this.startTimeMs() !== null && !this.roundEnded()
  );

  formattedTime = computed(() => {
    const total = this.elapsedSeconds();
    const minutes = Math.floor(total / 60).toString().padStart(2, '0');
    const seconds = (total % 60).toString().padStart(2, '0');
    return `${minutes}:${seconds}`;
  });

  constructor() {

    //Stop timer when round finish
    effect(() => {
      if (this.roundEnded()) {
        this.stopTimer();
        this._rqst.clearRequests();
      }
    });

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
      const startTime = this._round.startTimeSignal?.();

      if (gameId !== null && roundId !== null) {

        this.allowInternalNavigation = false;
        
        this._router.navigate(['/round', gameId, roundId], { replaceUrl: true });
        if (startTime) {
          this.startTimer(startTime);
        }
      }
    });

    effect(() => {
      this.logState("effect: ids changed");
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

    this._router.navigate(['']).then(ok => {
      if (ok) this._round.resetAll();
    });
  }

  toHomePageAfterDraw() {
    this._router.navigate(['']).then(ok => {
      if (ok) this._round.resetAll();
    });
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

    if (this.allowInternalNavigation) {
      this.allowInternalNavigation = false;
      return true;
    }

    const roundEnded = this.roundEnded();
    const winner = this.winner();                 
    const wonByForfeit = this.winnerByForfeit();  

    const hasWinner = winner !== null || wonByForfeit; 

    if (roundEnded && hasWinner) {
      this._game.endGame();  
      return true;
    }

    const confirmLeave = confirm(
      'If you leave now, the match will be forfeited. Do you want to continue?'
    );

    if (confirmLeave) {
      this._game.forfeitGame();
    }

    return confirmLeave;
  }

  openRequestPage() {
    this._dialog.open(RequestPage, {
      disableClose: true
    })
  }

  private startTimer(startTimeSeconds: number) {
    this.startTimeMs.set(startTimeSeconds * 1000);

    this.elapsedSeconds.set(
      Math.floor((Date.now() - this.startTimeMs()!) / 1000)
    );

    if (this.timerId) {
      clearInterval(this.timerId);
    }

    this.timerId = setInterval(() => {
      this.elapsedSeconds.set(
        Math.floor((Date.now() - this.startTimeMs()!) / 1000)
      );
    }, 1000);
  }

  private stopTimer() {
    if (this.timerId) {
      clearInterval(this.timerId);
      this.timerId = null;
    }
  }

  private logState(reason: string) {
    if (!this.debugEnabled) return;

    const snapshot = {
      reason,
      // ---- route ids ----
      gameId: this._round.gameId(),
      roundId: this._round.roundId(),

      // ---- round state ----
      roundEnded: this.roundEnded(),
      winner: this.winner(),
      winnerByForfeit: this.winnerByForfeit(),
      currentPlayer: this.currentPlayer(),
      mySymbol: this.mySimbol(),
      boardFilled: this.board().filter(c => c !== null).length,

      // ---- guard gates ----
      navigationAllowed: this._round.isNavigationAllowedSignal(),
      allowInternalNavigation: this.allowInternalNavigation,

      // ---- timer ----
      startTimeMs: this.startTimeMs(),
      elapsedSeconds: this.elapsedSeconds(),
      roundActive: this.roundActive(),
    };

    console.groupCollapsed(`[RoundPage] ${reason}`);
    console.table(snapshot);
    console.groupEnd();
  }


}
