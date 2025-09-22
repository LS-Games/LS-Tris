import { Component, Input, Output, EventEmitter } from '@angular/core';


@Component({
  selector: 'app-board',
  standalone: true,
  imports: [],
  templateUrl: './board.html',
  styleUrl: './board.scss'
})

export class Board {
  @Input() board:string[] = Array(9).fill('@')
  @Output() move = new EventEmitter<number>();

  onCellClick(index: number) {
    if(this.board[index] === '@') {
      this.move.emit(index);
    }
  }

}
