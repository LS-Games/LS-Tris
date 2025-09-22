import { Component, inject } from '@angular/core';
import { DialogRef } from '@angular/cdk/dialog'; // Import DialogRef to manage dialog reference
import { RouterLink } from '@angular/router'; 
import { ReactiveFormsModule, FormBuilder, FormGroup, Validators } from '@angular/forms';

@Component({
  selector: 'app-signin-form',
  imports: [RouterLink, ReactiveFormsModule],
  templateUrl: './signin-form.html',
  styleUrl: './signin-form.scss'
})
export class SigninForm {
  /* When we created the dialog in header.ts, Angular automatically provided a DialogRef instance for this component.
    to manage it we have to inject a DialogRef, after which we can create a function to close it, for example */
  private readonly _dialogRef = inject(DialogRef<SigninForm>); // Inject DialogRef to control the dialog

  //Create a reactive form here using FormBuilder
  private readonly _formBuilder = inject(FormBuilder);
  buttonClicked = false;

  signinForm : FormGroup;

  constructor() { 
    this.signinForm = this._formBuilder.group ({
      nickname: ['', [Validators.required, Validators.minLength(3)]], 
      email: ['', [Validators.required, Validators.email]], 
      password: ['', [Validators.required, Validators.minLength(6)]]
    });
  }
  
  close() {
    this._dialogRef.close(); // Close the dialog when the close button is clicked
  }

  onSubmit() {
    this.buttonClicked = true;
  }







  

}
