import { Component, inject } from '@angular/core';
import { Dialog, DialogRef } from '@angular/cdk/dialog';
import { ReactiveFormsModule, FormBuilder, Validators } from '@angular/forms';
import { SignupForm } from '../signup-form/signup-form';
import { AuthService } from '../../core/services/auth.service';
import { NotificationService } from '../../core/services/notification';
import { takeUntilDestroyed } from '@angular/core/rxjs-interop';
import { DestroyRef } from '@angular/core';

@Component({
  selector: 'app-login-form',
  imports: [ReactiveFormsModule],
  templateUrl: './login-form.html',
  styleUrl: './login-form.scss'
})
export class LoginForm {

  private readonly _dialogRef = inject(DialogRef<LoginForm>);
  private readonly _dialog = inject(Dialog);
  private readonly _formBuilder = inject(FormBuilder);
  private readonly _authService = inject(AuthService);
  private readonly _notificationService = inject(NotificationService);
  private readonly _destroyRef = inject(DestroyRef);

  buttonClicked = false;

  loginForm = this._formBuilder.group({
    nickname: ['', [Validators.required]],
    password: ['', [Validators.required, Validators.minLength(6)]],
  });

  constructor() {

    // Listen for login success
    this._authService.onLoginSuccess()
      .pipe(takeUntilDestroyed(this._destroyRef))
      .subscribe((id) => {

        const nickname = this.loginForm.value.nickname;
        
        this._notificationService.show('success', `Welcome back! ${nickname}`, 4000);
        this.close();
        this.buttonClicked = false;
      });

    // Listen for login error
    this._authService.onLoginError()
      .pipe(takeUntilDestroyed(this._destroyRef))
      .subscribe((err) => {
        this._notificationService.show('error', err, 4000);
        this.buttonClicked = false;
      });
  }

  // Closes the login dialog 
  close() {
    this._dialogRef.close();
  }

  // Switch to the signup form 
  gotoSignIn() {
    this._dialogRef.close();
    this._dialog.open(SignupForm, { disableClose: true });
  }

  // Handles login form submission 
  onSubmit() {
    if (this.buttonClicked) return;
    this.buttonClicked = true;

    if (!this.loginForm.valid) {
      this._notificationService.show('warning', 'Please fill in all required fields.');
      this.buttonClicked = false;
      return;
    }

    const { nickname, password } = this.loginForm.value;

    // Call AuthService to sign in via WebSocket
    this._authService.signin(nickname!, password!);
  }
}
