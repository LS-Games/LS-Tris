import { Component, Input } from '@angular/core';

@Component({
  selector: 'app-game-card',
  standalone: true,
  imports: [],
  templateUrl: './game-card.html',
  styleUrl: './game-card.scss'
})

export class GameCard {
  @Input() id!:number;
  @Input() creator!:string;
  @Input() data!:string;
  @Input() owner!:string;
  @Input() state!: 'new' | 'active' | 'waiting' | 'finished'
  @Input() current_streak?: number;
}




 