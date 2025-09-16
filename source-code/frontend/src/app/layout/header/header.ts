import { Component, inject } from '@angular/core';
import { RouterLink, RouterLinkActive } from '@angular/router';
import { CommonModule } from '@angular/common';
import { AuthService } from '../../core/auth/auth.service';
import { Dialog } from '@angular/cdk/dialog';
import { SigninForm } from '../../pages/signin-form/signin-form';
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

  protected openSignIn() {
    this.dialog.open(SigninForm, {
      disableClose: true // Prevent closing the dialog by clicking outside or pressing ESC
    });
  }
  
  protected openLogin() {
    this.dialog.open(LoginForm, {
      disableClose: true
    });
  }
}