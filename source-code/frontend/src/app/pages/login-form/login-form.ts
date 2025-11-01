import { Component, inject } from '@angular/core';
import { Dialog, DialogRef } from '@angular/cdk/dialog';
import { ReactiveFormsModule, FormBuilder, FormGroup, Validators } from '@angular/forms';
import { SignupForm } from '../signup-form/signup-form';
import { WebsocketService } from '../../core/services/websocket.service';
import { NotificationService } from '../../core/services/notification';
import { AuthService } from '../../core/services/auth.service';

@Component({
  selector: 'app-login-form',
  imports: [ReactiveFormsModule],
  templateUrl: './login-form.html',
  styleUrl: './login-form.scss'
})
export class LoginForm {

  // Reference to the current dialog (used to close it)
  private readonly _dialogRef = inject(DialogRef<LoginForm>);

  // Used to open the Signup form dialog
  private readonly _dialog = inject(Dialog);

  // Angular FormBuilder service used for creating reactive forms
  private readonly _formBuilder = inject(FormBuilder);

  // Custom WebSocket service for real-time communication with the bridge
  private readonly _ws = inject(WebsocketService);

  // NotificationService allows us to show toast messages in the UI
  private readonly _notificationService = inject(NotificationService);

  // AuthService allows us to change the users's status to logged in
  private readonly _authService = inject(AuthService);

  // Used to show button loading or disable it after click
  buttonClicked = false;

  // Reactive form instance
  loginForm: FormGroup;

  constructor() {
    // Create and initialize the form structure with validators
    this.loginForm = this._formBuilder.group({
      nickname: ['', [Validators.required]],
      password: ['', [Validators.required, Validators.minLength(6)]]
    });
  }

  // Close the login dialog
  close() {
    this._dialogRef.close();
  }

  // Switch from Login dialog → Signup dialog
  gotoSignIn() {
    this._dialogRef.close();
    this._dialog.open(SignupForm, {
      disableClose: true // Prevent user from closing it by clicking outside
    });
  }

  // Called when user submits the login form
  onSubmit() {
    this.buttonClicked = true;

    // Check if all form fields are valid before proceeding
    if (!this.loginForm.valid) {
      this._notificationService.show('warning', 'Please fill in all required fields.');
      return;
    }

    // Extract values from the form
    const { nickname, password } = this.loginForm.value;

    // Open the WebSocket connection to the bridge (persistent)
    this._ws.connect('ws://localhost:3002')

    //We can use .then because the function return a promise
      .then(() => {
        console.log('Connected to WebSocket');

        // Register the event listener for messages from backend
        // We do this *before* sending to make sure we don't miss any response
        this._ws.onMessage((event: MessageEvent) => {
          try {
            // Parse first-level JSON sent by the bridge
            const wrapper = JSON.parse(event.data);

            // Parse the backend response sent inside backendResponse
            const backend = JSON.parse(wrapper.backendResponse);

            console.log('Backend response:', backend);

            // Check the response status and show the proper notification
            if (backend.status === 'success') {
              this._notificationService.show('success', backend.message, 4000);
              this._authService.login();

              this.close(); // Close dialog on successful login
            } else if (backend.status === 'error') {
              this._notificationService.show('error', backend.error_message, 4000);
            }

          } catch (err) {
            console.error('Failed to parse backend response:', err);
          }
        });

        // Create the payload to send to backend via WebSocket
        const payload = {
          action: 'player_signin',
          nickname,
          password
        };

        // Send the payload to backend through the bridge
        this._ws.send(payload);
        console.log('Payload sent to backend:', payload);

      })
      .catch((err) => {
        console.error('WebSocket connection failed:', err);
        this._notificationService.show('error', 'Unable to connect to the server. Please try again later.');
      });
  }
}
