import { Component, inject } from '@angular/core';
import { RouterLink, RouterLinkActive } from '@angular/router';
import { CommonModule } from '@angular/common';
import { AuthService } from '../../core/auth/auth.service';
import { Dialog } from '@angular/cdk/dialog';
import { SignupForm } from '../../pages/signup-form/signup-form';
import { LoginForm } from '../../pages/login-form/login-form';


@Component({
  selector: 'app-header',
  standalone: true,
  imports: [RouterLink, RouterLinkActive, CommonModule],
  templateUrl: './header.html',
  styleUrls: ['./header.scss']
})

export class Header {
  auth = inject(AuthService); // static access to the AuthService

  logout() {
    this.auth.logout()
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