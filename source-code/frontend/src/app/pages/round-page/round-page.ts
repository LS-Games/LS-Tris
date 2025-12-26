import { Component, inject, effect } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BoardComponent } from '../round-page/components/board/board';
import { RoundService } from '../../core/services/round.service';
import { GameService } from '../../core/services/game.service';
import { Router } from '@angular/router';
@Component({
  selector: 'app-round-page',
  standalone: true,
  imports: [CommonModule, BoardComponent],
  templateUrl: './round-page.html',
  styleUrl: './round-page.scss'
})
export class RoundPage {

  private readonly _round = inject(RoundService);
  private readonly _router = inject(Router);
  private readonly _game = inject(GameService);
  
  player1Nickname = this._round.player1Nickname;
  player2Nickname = this._round.player2Nickname;

  board = this._round.boardSignal;
  currentPlayer = this._round.currentPlayerTurnSignal;
  mySimbol = this._round.mySymbolSignal;
  winner = this._round.winnerSignal;
  roundEndend = this._round.roundEndedSignal;

  constructor() {

      effect(() => {
        const winner = this.winner();
        if (winner) {
          console.log("Round finito:", winner);
        }
      });
  }

  handleMove(index:number) {

    if(this.currentPlayer() !== this.mySimbol()) {
      console.log("Non è il tuo turno");
    }

    if(this.board()[index] !== null) return;

    this._round.makeMove(index);
  }

  toHomePage() {
    this._round.reset();
    this._router.navigate(['']);
  }

  newGameAfterWin() {

  }

  gameEndAfterWin() {
    this._game.endGame();
    this.toHomePage();
  }
}
