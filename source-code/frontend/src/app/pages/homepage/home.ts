import { Component, inject } from '@angular/core';
import { AuthService } from '../../core/services/auth.service';
import { Dialog } from '@angular/cdk/dialog';
import { LoginForm } from '../login-form/login-form';
import { Lobby} from '../lobby/lobby'

@Component({
  selector: 'app-home',
  standalone: true,
  imports: [],
  templateUrl: './home.html',
  styleUrls: ['./home.scss']
})

export class Home {
  auth = inject(AuthService); 

  private readonly _dialog = inject(Dialog);

  protected openLogin() {
    this._dialog.open(LoginForm, {
      disableClose: true
    });
  }

  protected openLobby() {
    this._dialog.open(Lobby, {
      disableClose: true
    });

    

  }

  
}
