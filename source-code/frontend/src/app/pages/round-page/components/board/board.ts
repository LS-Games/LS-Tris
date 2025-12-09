import { Component, Input, Output, EventEmitter } from '@angular/core';

@Component({
  selector: 'app-board',
  standalone: true,
  templateUrl: './board.html',
  styleUrls: ['./board.scss'],
})
export class BoardComponent {
  
  @Input() board: (null | 'X' | 'O')[] = [];
  @Input() winner: string | null = null;

  @Output() move = new EventEmitter<number>();
  @Output() restartGame = new EventEmitter<void>();

  playMove(i: number) {
    if (this.winner) return;
    this.move.emit(i);
  }

  restart() {
    this.restartGame.emit();
  }
}
