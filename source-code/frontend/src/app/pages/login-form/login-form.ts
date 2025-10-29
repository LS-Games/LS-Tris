import { Component, inject } from '@angular/core';
import { Dialog, DialogRef } from '@angular/cdk/dialog';
import { SignupForm } from '../signup-form/signup-form';
import { ReactiveFormsModule, FormBuilder, FormGroup, Validators } from '@angular/forms';
import { AuthService } from '../../core/services/auth.service';

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
  private readonly _auth = inject(AuthService);
  buttonClicked = false;

  loginForm: FormGroup; // it is a variable that will hold the form structure and its state

  constructor() {
    this.loginForm = this._formBuilder.group({                        //FormBuilder has a method called group() that creates a FormGroup instance
      email: ['', [Validators.required, Validators.email]],           //Each form control is represented as a key-value pair in the object passed to group()
      password: ['', [Validators.required, Validators.minLength(6)]]  //The first element is the default value (we want an empty field), the second is an array of validators
    });
  }

  close() {
    this._dialogRef.close(); 
  }
  
  gotoSignIn() {
    this._dialogRef.close(); 
    this._dialog.open(SignupForm, { 
      disableClose: true 
    });
  } 

  onSubmit() {
    this.buttonClicked = true;
    if(this.loginForm.valid) { //We check if the form is valid
      const { email, password } = this.loginForm.value; //We extract the email and password from the form values
      console.log('Dati catturati:', email, password);

      //##########TEST##########
      // this._auth.login(email, password).subscribe({
      //   next: (res) => {
      //     console.log('Login effettuato con successo', res);
      //     localStorage.setItem('token', res.token); //We store the token in LocalStorage
      //     this.close(); //We close the dialog
      //   },

      //   error: (err) => {
      //     console.error('Errore durante il login', err);
      //   }
      // }); 
    }
  }
}
