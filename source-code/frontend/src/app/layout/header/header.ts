import { Component, inject } from '@angular/core';
import { RouterLink, RouterLinkActive } from '@angular/router';
import { CommonModule } from '@angular/common';
import { AuthService } from '../../core/services/auth.service';
import { Dialog } from '@angular/cdk/dialog';
import { SignupForm } from '../../pages/signup-form/signup-form';
import { LoginForm } from '../../pages/login-form/login-form';
import { Router } from '@angular/router';


@Component({
  selector: 'app-header',
  standalone: true,
  imports: [RouterLink, RouterLinkActive, CommonModule],
  templateUrl: './header.html',
  styleUrls: ['./header.scss']
})

export class Header {
  auth = inject(AuthService); 
  private readonly _router = inject(Router);

  logout() {
    this.auth.logout()
    this._router.navigate(['/']);
  }

  private readonly dialog = inject(Dialog);

  protected openSignUp() {
    this.dialog.open(SignupForm, {
      disableClose: true // Prevent closing the dialog by clicking outside or pressing ESC
    });
  }
  
  protected openLogin() {
    this.dialog.open(LoginForm, {
      disableClose: true
    });
  }
}