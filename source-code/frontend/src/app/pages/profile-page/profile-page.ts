import { Component, computed, inject } from '@angular/core';
import { AuthService } from '../../core/services/auth.service';
import { CommonModule } from '@angular/common';

@Component({
  selector: 'app-profile-page',
  standalone: true,
  imports: [CommonModule],
  templateUrl: './profile-page.html',
  styleUrl: './profile-page.scss'
})
export class ProfilePage {

  constructor() {
    this._auth.ensureUserLoaded();
  }

  private readonly _auth = inject(AuthService);

  user = this._auth.currentUser;

  isLoading = computed(() => this.user() === null);
}
