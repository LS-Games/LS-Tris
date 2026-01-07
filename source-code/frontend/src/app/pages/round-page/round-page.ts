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

  private allowInternalNavigation = false;
  private exitGameIntent = false;
  
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

    //Start timer when round begin
    // effect(() => {
    //   const startTime = this._round.startTimeSignal?.();

    //   if (startTime && this.startTimeMs() === null) {
    //     this.startTimer(startTime);
    //   }
    // });

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
        this._router.navigate(['/round', gameId, roundId], { replaceUrl: true });
        if (startTime) {
          this.startTimer(startTime);
        }
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
    this.exitGameIntent = true;
    this._round.resetAll();
    this._router.navigate(['']);
  }

  toHomePageAfterDraw() {
    this.exitGameIntent = true;

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

  // Navigazioni interne “lecite” gestite dalla tua logica (rematch, ecc.)
  if (this._round.isNavigationAllowedSignal()) {
    return true;
  }

  // Click su pulsanti interni dove vuoi bypassare il guard
  if (this.allowInternalNavigation) {
    this.allowInternalNavigation = false;
    return true;
  }

  const roundEnded = this.roundEnded();
  const winner = this.winner();                 // 'X' | 'O' | null
  const wonByForfeit = this.winnerByForfeit();  // boolean

  const hasWinner = winner !== null || wonByForfeit; // vittoria “vera” o per forfeit

  // ✅ CASO 1: round finito con vincitore → niente popup, niente forfeit
  if (roundEnded && hasWinner) {
    this._game.endGame();   // setta solo FINISHED
    return true;
  }

  // ✅ CASO 2: round finito in pareggio → se esci perdi a tavolino
  // ✅ CASO 3: round non finito → se esci perdi a tavolino
  const confirmLeave = confirm(
    'If you leave now, the match will be forfeited. Do you want to continue?'
  );

  if (confirmLeave) {
    // Qui dentro siamo:
    // - oppure in pareggio
    // - oppure a round in corso
    // In entrambi i casi: uscire = perdere a tavolino
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

}
