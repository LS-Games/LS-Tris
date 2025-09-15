import { Component, inject } from '@angular/core';
import { RouterLink, RouterLinkActive } from '@angular/router';
import { CommonModule } from '@angular/common';
import { AuthService } from '../../core/auth/auth.service';
import { Dialog } from '@angular/cdk/dialog';
import { SigninForm } from '../../pages/signin-form/signin-form';


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

  private dialog = inject(Dialog);
  protected openModal() {
    this.dialog.open(SigninForm);
  }
}