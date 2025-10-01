import { Component, inject } from '@angular/core';
import { Dialog, DialogRef } from '@angular/cdk/dialog';
import { GameCard } from '../lobby/components/game-card/game-card';

@Component({
  selector: 'app-lobby',
  standalone: true,
  imports: [GameCard],
  templateUrl: './lobby.html',
  styleUrl: './lobby.scss'
})
export class Lobby {

  private readonly _dialogRef = inject(DialogRef<Lobby>);
  private readonly _dialog = inject(Dialog)

  matches = [
    { id: 1, creator: 'Luca', data: new Date().toLocaleDateString('it-IT'), owner: 'Luca', state: 'waiting' as const, current_streak: 3 },
    { id: 2, creator: 'Marco', data: new Date().toLocaleDateString('it-IT'), owner: 'Paolo', state: 'active' as const, current_streak: 5 },
    { id: 3, creator: 'Giulia', data: new Date().toLocaleDateString('it-IT'), owner: 'Maria', state: 'new' as const, current_streak: 0 },
    { id: 4, creator: 'Franco', data: new Date().toLocaleDateString('it-IT'), owner: 'Peppe', state: 'finished' as const, current_streak: 0 },
  ];

  close() {
    this._dialogRef.close();
  }
}
