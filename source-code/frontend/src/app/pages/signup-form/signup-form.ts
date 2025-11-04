import { Component, inject } from '@angular/core';
import { DialogRef } from '@angular/cdk/dialog';
import { RouterLink } from '@angular/router';
import { ReactiveFormsModule, FormBuilder, Validators } from '@angular/forms';
import { NotificationService } from '../../core/services/notification';
import { AuthService } from '../../core/services/auth.service';

@Component({
  selector: 'app-signup-form',
  imports: [RouterLink, ReactiveFormsModule],
  templateUrl: './signup-form.html',
  styleUrl: './signup-form.scss'
})
export class SignupForm {

  // Reference to the dialog instance created by the parent component.
  private readonly _dialogRef = inject(DialogRef<SignupForm>);

  // Service responsible for handling authentication logic and backend communication.
  private readonly _auth = inject(AuthService);

  // Service used to display success, warning, or error notifications in the UI.
  private readonly _notification = inject(NotificationService);

  // Utility service for building and managing Angular reactive forms.
  private readonly _formBuilder = inject(FormBuilder);

  // Used to prevent multiple form submissions (button disable state).
  buttonClicked = false;

  // Reactive form definition and validation rules.
  signupForm = this._formBuilder.group({
    nickname: ['', [Validators.required, Validators.minLength(3)]],
    email: ['', [Validators.required, Validators.email]],
    password: ['', [Validators.required, Validators.minLength(6)]],
    accepted_policy: [false, Validators.requiredTrue],
  });

  /**
   * Closes the signup dialog window.
   */
  close() {
    this._dialogRef.close();
  }

  /**
   * Handles the signup form submission.
   * Validates input, sends a signup request through AuthService,
   * and displays notifications based on the backend response.
   */
  onSubmit() {
    this.buttonClicked = true;

    // Abort if the form is invalid.
    if (this.signupForm.invalid) {
      console.warn('Signup form is invalid.');
      this.buttonClicked = false;
      return;
    }

    // Extract values from the form.
    const { nickname, email, password } = this.signupForm.value;

    // Delegate the signup logic to AuthService (uses HTTP bridge internally).
    this._auth.signup(nickname!, email!, password!).subscribe({

      /**
       * Executed when the HTTP request completes successfully.
       * The response structure contains a backendResponse JSON string.
       */
      next: (response: any) => {
        if (response?.backendResponse) {
          const backend = JSON.parse(response.backendResponse);
          console.log('Backend response:', backend);

          if (backend.status === 'success') {
            // Signup successful — show notification and close the dialog.
            this._notification.show('success', backend.message || 'Signup successful.', 4000);
            this.close();
          } else {
            // Backend returned an error status.
            this._notification.show('error', backend.error_message || 'Signup failed.', 4000);
          }
        }
        this.buttonClicked = false;
      },

      /**
       * Executed if the HTTP request fails (network or bridge issue).
       */
      error: (err) => {
        console.error('Error communicating with backend:', err);
        this._notification.show('error', 'Signup failed — please try again later.');
        this.buttonClicked = false;
      }
    });
  }
}
