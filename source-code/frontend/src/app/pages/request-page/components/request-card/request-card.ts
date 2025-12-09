import { Component, inject, Input } from '@angular/core';
import { DatePipe } from '@angular/common';
import { RequestsService } from '../../../../core/services/requests.service';

@Component({
  selector: 'app-request-card',
  imports: [DatePipe],
  templateUrl: './request-card.html',
  styleUrl: './request-card.scss'
})

export class RequestCard {

  private readonly _rqst = inject(RequestsService);

  @Input() sender_nickname!:string;
  @Input() data!:string;
  @Input() id_request!:number;

  rejectParticipationRequest() {
    this._rqst.rejectParticipationRequest(this.id_request);
  }

  acceptParticipationReqeust() {
    this._rqst.acceptParticipationReqeust(this.id_request);
  }
  
}


