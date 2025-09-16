import { Component, inject } from '@angular/core';
import { DialogRef } from '@angular/cdk/dialog'; // Import DialogRef to manage dialog reference
import { RouterLink } from '@angular/router'; 

@Component({
  selector: 'app-signin-form',
  imports: [RouterLink],
  templateUrl: './signin-form.html',
  styleUrl: './signin-form.scss'
})
export class SigninForm {
  /* When we created the dialog in header.ts, Angular automatically provided a DialogRef instance for this component.
    to manage it we have to inject a DialogRef, after which we can create a function to close it, for example */
  private readonly _dialogRef = inject(DialogRef<SigninForm>); // Inject DialogRef to control the dialog

  close() {
    this._dialogRef.close(); // Close the dialog when the close button is clicked
  }

  

}
