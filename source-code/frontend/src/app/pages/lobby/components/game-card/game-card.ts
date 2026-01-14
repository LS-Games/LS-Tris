import { Component, inject, Input } from '@angular/core';
import { DatePipe } from '@angular/common';
import { RequestsService } from '../../../../core/services/requests.service';
import { AuthService } from '../../../../core/services/auth.service';

@Component({
  selector: 'app-game-card',
  standalone: true,
  imports: [DatePipe],
  templateUrl: './game-card.html',
  styleUrl: './game-card.scss'
})

export class GameCard {

  private readonly rqst_service = inject(RequestsService);
  private readonly auth = inject(AuthService);

  @Input() id!:number;
  @Input() creator!:string;
  @Input() data!:string;
  @Input() owner!:string;
  @Input() state!:string
  @Input() owner_current_streak?: number;

  sendGameRequest() {
    const playerId = this.auth.id;

    if(playerId == null) {
      console.warn('User not logged in, cannot send request');
      return;
    }
    
    this.rqst_service.requestParticipation(this.id, playerId);

  }
}






 