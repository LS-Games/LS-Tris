import { Component, Input } from '@angular/core';
import { DatePipe } from '@angular/common';

@Component({
  selector: 'app-request-card',
  imports: [DatePipe],
  templateUrl: './request-card.html',
  styleUrl: './request-card.scss'
})

export class RequestCard {

  @Input() sender_nickname!:string;
  @Input() data!:string;
  
}
