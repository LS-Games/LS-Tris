import { Component, inject } from '@angular/core';
import { DialogRef } from '@angular/cdk/dialog';
import { RouterLink } from '@angular/router'; 
import { ReactiveFormsModule, FormBuilder, FormGroup, Validators } from '@angular/forms';
import { HttpClient } from '@angular/common/http';

@Component({
  selector: 'app-signup-form',
  imports: [RouterLink, ReactiveFormsModule],
  templateUrl: './signup-form.html',
  styleUrl: './signup-form.scss'
})
export class SignupForm {

  private readonly http = inject(HttpClient);
  // When we created the dialog in header.ts, Angular automatically provided a DialogRef instance for this component.
  // To manage it we have to inject a DialogRef, after which we can create a function to close it, for example. 
  private readonly _dialogRef = inject(DialogRef<SignupForm>);

  // Create a reactive form using FormBuilder
  private readonly _formBuilder = inject(FormBuilder);
  buttonClicked = false;

  signupForm: FormGroup;

  constructor() { 
    this.signupForm = this._formBuilder.group({
      nickname: ['', [Validators.required, Validators.minLength(3)]],
      email: ['', [Validators.required, Validators.email]],
      password: ['', [Validators.required, Validators.minLength(6)]]
    });
  }
  
  close() {
    this._dialogRef.close();
  }

  onSubmit() {
    this.buttonClicked = true;
    if (this.signupForm.valid) {
      
      const payload = {
        action: 'player_signup',
        nickname: this.signupForm.value.nickname,
        email: this.signupForm.value.email,
        password: this.signupForm.value.password
      };

      //We use this http function to send payload to bridge
      //The http function return a Observable
      this.http.post('http://localhost:3001/api/send', { message: JSON.stringify(payload) })

        //It is similar to .then()
        .subscribe({

          //next is executed when the request is successful 
          next: (response: any) => {
            console.log('Backend response:', response.backendResponse)
          // Optionally show a success message or close dialog
            alert('Signup successful!');
            this.close();
          },

          error: (err) => {
            console.error('Error communicating with backend:', err);
            alert('Signup failed — please try again later.');
          }
        });

    } else {
      console.warn('Signup form is invalid.');
    }
  }
}
