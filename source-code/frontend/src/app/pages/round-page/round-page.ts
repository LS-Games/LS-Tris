import { Component, Input, signal } from '@angular/core';
import { CommonModule } from '@angular/common';
import { Board } from '../round-page/components/board/board'

@Component({
  selector: 'app-round-page',
  standalone: true,
  imports: [CommonModule, Board],
  templateUrl: './round-page.html',
  styleUrl: './round-page.scss'
})

export class RoundPage {

  @Input() player1:string = '';
  @Input() player2:string = '';

  board:string[] = Array(9).fill('@');
  currentPlayer: 'X' | 'O' = 'X' //Union type, it means that it can be only X or only O, the initial value is X

  onMove(index: number) {
    if (this.board[index] === '@') {
      this.board[index] = this.currentPlayer;
      this.currentPlayer = this.currentPlayer === 'X' ? 'O' : 'X';
    }
  }
}
