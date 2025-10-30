import { Component, inject, OnDestroy } from '@angular/core';
import { Dialog, DialogRef } from '@angular/cdk/dialog';
import { SignupForm } from '../signup-form/signup-form';
import { ReactiveFormsModule, FormBuilder, FormGroup, Validators } from '@angular/forms';
import { AuthService } from '../../core/services/auth.service';
import { WebsocketService } from '../../core/services/websocket.service';

@Component({
  selector: 'app-login-form',
  imports: [ReactiveFormsModule],
  templateUrl: './login-form.html',
  styleUrl: './login-form.scss'
})
export class LoginForm {

  // Reference to the current dialog (used to close it)
  private readonly _dialogRef = inject(DialogRef<LoginForm>); 

  // Used to open other dialogs (e.g. the signup form)
  private readonly _dialog = inject(Dialog);

  // Angular FormBuilder service for building reactive forms
  private readonly _formBuilder = inject(FormBuilder);

  // Authentication service (for potential HTTP-based login)
  private readonly _auth = inject(AuthService);

  // Custom WebSocket service used to communicate with the bridge in real time
  private readonly _ws = inject(WebsocketService);

  // Used to disable the button or show loading feedback during submission
  buttonClicked = false;

  // Holds the login form structure and its validation state
  loginForm: FormGroup;

  constructor() {
    // Create a reactive form using the FormBuilder
    this.loginForm = this._formBuilder.group({
      // First field: email — must be required and in valid email format
      email: ['', [Validators.required, Validators.email]],

      // Second field: password — required and must have at least 6 characters
      password: ['', [Validators.required, Validators.minLength(6)]]
    });
  }

  // Function to close the dialog when user cancels or login is successful
  close() {
    this._dialogRef.close(); 
  }
  
  // Function to navigate from login → signup dialog
  gotoSignIn() {
    this._dialogRef.close(); // close the current dialog
    this._dialog.open(SignupForm, { 
      disableClose: true // prevent user from closing the signup dialog by clicking outside
    });
  } 

  // Called when the user submits the login form
  onSubmit() {
    this.buttonClicked = true; // mark that the login button was pressed

    // Check if all form fields are valid before sending data
    if (this.loginForm.valid) {
      // Extract values from the reactive form
      const { email, password } = this.loginForm.value;
      console.log('Captured data:', email, password);

      // Open a WebSocket connection to the bridge (Node.js server)
      this._ws.connect('ws://localhost:3002');

      // Wait briefly to ensure the connection is established
      setTimeout(() => {

        const payload = {
          action: 'player_signin',  // tells the backend what kind of action this is
          email: email,             // user’s email
          password: password        // user’s password
        };

        // Send login credentials to the bridge through the WebSocket
        this._ws.send(payload);

      }, 500); // slight delay before sending to allow socket to open fully
    }

    // Listen for responses from backend
      this._ws.onMessage((event: MessageEvent) => {
        try {
          const data = JSON.parse(event.data);
          const backend = JSON.parse(data.backendResponse);

          console.log('📩 Backend says:', backend);

          if (backend.status === 'success') {
            alert('✅ Login successful!');
            this.close();
          } else {
            alert('❌ Login failed.');
          }

        } catch (err) {
          console.error('Failed to parse backend response:', err);
        }
      });
  }

  // Lifecycle hook: automatically called when the component is destroyed
  // We use it to close the WebSocket connection cleanly
  ngOnDestroy() {
    this._ws.close();
  }
}
