import { Component, inject } from '@angular/core';
import { DialogRef } from '@angular/cdk/dialog';
import { RouterLink } from '@angular/router'; 
import { ReactiveFormsModule, FormBuilder, FormGroup, Validators } from '@angular/forms';
import { SocketService } from '../../core/auth/socket.service';

@Component({
  selector: 'app-signup-form',
  imports: [RouterLink, ReactiveFormsModule],
  templateUrl: './signup-form.html',
  styleUrl: './signup-form.scss'
})
export class SignupForm {

  private readonly socketService = inject(SocketService);
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

    //This function listens to the eventListener in onMessage() function 
    //When the event arrive (server response) it executes the function '(data) => {...}'
    this.socketService.onMessage((data) => {
        console.log(data);
    })
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

      this.socketService.send(payload);

      //DEBUG
      console.log('JSON inviato al server:', JSON.stringify(payload));
      
    }
  }
}
