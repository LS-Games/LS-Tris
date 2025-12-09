import { Component, inject, Input, signal, effect } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BoardComponent } from '../round-page/components/board/board';
import { RoundService } from '../../core/services/round.service';

@Component({
  selector: 'app-round-page',
  standalone: true,
  imports: [CommonModule, BoardComponent],
  templateUrl: './round-page.html',
  styleUrl: './round-page.scss'
})
export class RoundPage {

  private readonly _round = inject(RoundService);

  player1Nickname = this._round.player1Nickname;
  player2Nickname = this._round.player2Nickname;

  board = this._round.boardSignal;
  currentPlayer = this._round.currentPlayerTurnSignal;
  mySimbol = this._round.mySymbolSignal;
  winner = this._round.winnerSignal;

  constructor() {

      effect(() => {
        const winner = this.winner();
        if (winner) {
          console.log("Round finito:", winner);
        }
      });
  }

  handleMove(index:number) {

    if(this.winner()) return;

    if(this.currentPlayer() !== this.mySimbol()) {
      console.log("Non è il tuo turno");
    }

    if(this.board()[index] !== null) return;

    this._round.makeMove(index);
  }

}
