import { Component, Input, Output, EventEmitter } from '@angular/core';

@Component({
  selector: 'app-board',
  standalone: true,
  templateUrl: './board.html',
  styleUrls: ['./board.scss'],
})
export class BoardComponent {
  
  @Input() board: (null | 'X' | 'O')[] = [];

  @Output() move = new EventEmitter<number>();
  @Output() restartGame = new EventEmitter<void>();

  playMove(i: number) {
    this.move.emit(i);
  }

}
