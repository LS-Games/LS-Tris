/*
  This service tracks the client login status and the number of invitations received. 
  We use Angular Signals which are small reactive value boxes 

  Injectable -> It means that this class can be injected
  signal -> It creates a signal that is a value which notifies the user when it changes
  computed -> It creates a derived Signal that depends on other signals 
  effect -> It executes a function each time the signals in that function change (collateral effects)

  Example: User login -> _isLogged set -> isLogged set -> effect() -> set token -> User logout -> _isLogged set -> isLogged set -> effect -> set token
*/ 

import { Injectable, signal, computed, effect } from '@angular/core';

@Injectable({ providedIn: 'root' }) // This means that there is only one global instance of this service available throughout the entire app 
export class AuthService {

  private _isLoggedIn = signal<boolean>(false); // It creates a private boolean signal that is false by default (the _ before the word remind us that it's private)
  readonly isLoggedIn = computed(() => this._isLoggedIn());  //We want the computed function to update itself when _isLoggedIn change, we use readonly because to change the state we use login() and logout() function

  private _invites = signal<number>(0); //We use the same isLoggedIn logic here
  readonly invites = computed(() => this._invites());

  constructor() {
    // The constructor begins when the app creates the service
    const token = localStorage.getItem('token'); //We retrive the token which is located in the LocalStore of the Browser
    this._isLoggedIn.set(!!token); //We use !! (double NOT) to convert it in boolean type, because token alone would be a string value 

    // The effect executes the functions instantly and each time that an inner signal changes (in this case when this.isLoggedIn() changes)
    effect(() => {
      if (this.isLoggedIn()) localStorage.setItem('token', 'demo'); //If the user is logged in save "demo" in the "token" key (WE WILL SET THE REAL TOKEN LOGIC HERE)
      else localStorage.removeItem('token');  //Else remove it
    });
  }

  login() {
    this._isLoggedIn.set(true); //This will trigger the effect and the token will be written in LocalStorage
    this._invites.set(3); 
  }

  logout() {
    this._isLoggedIn.set(false);
    this._invites.set(0);
  }
}