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

  /**
   * We're using async and await because we have to wait to know if the 
   * navigation is blocked, whitout the await the logout would happen immediately
   * when perhaps it should not.
   */

  async logout() {

    const currentUrl = this._router.url;

    if (currentUrl === '/') {
      this.auth.logout();
      return;
    }
    
    const navigated = await this._router.navigate(['/']);

    if (!navigated) return;

    this.auth.logout();
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