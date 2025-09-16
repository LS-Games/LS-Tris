import { Component, inject } from '@angular/core';
import { AuthService } from '../../core/auth/auth.service';
import { LoginForm } from '../login-form/login-form';
import { Dialog } from '@angular/cdk/dialog';

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

  
}
