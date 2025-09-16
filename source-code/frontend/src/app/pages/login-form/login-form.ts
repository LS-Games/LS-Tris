import { Component, inject } from '@angular/core';
import { Dialog, DialogRef } from '@angular/cdk/dialog';
import { SigninForm } from '../signin-form/signin-form';

@Component({
  selector: 'app-login-form',
  imports: [],
  templateUrl: './login-form.html',
  styleUrl: './login-form.scss'
})
export class LoginForm {
  private readonly _dialogRef = inject(DialogRef<LoginForm>); 
  private readonly _dialog = inject(Dialog);

  close() {
    this._dialogRef.close(); 
  }
  
  gotoSignIn() {
    this._dialogRef.close(); 
    this._dialog.open(SigninForm);
  } 
}
