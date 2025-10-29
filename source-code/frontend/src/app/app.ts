import { Component, inject } from '@angular/core';
import { RouterOutlet } from '@angular/router';
import { Header } from './layout/header/header';
import { Footer } from './layout/footer/footer';
import { NotificationsComponent } from './features/notifications/notifications';
// import { NotificationService } from './core/services/notification';

@Component({
  selector: 'app-root',
  standalone: true,
  imports: [RouterOutlet, Header, Footer, NotificationsComponent],
  templateUrl: './app.html',
  styleUrls: ['./app.scss']
})

export class App {
  //TEST NOTIFICATION
  // private readonly notificationService = inject(NotificationService);

  // constructor() {
  //   this.notificationService.show('success', 'Registrazione completata.', 10000);
  //   this.notificationService.show('error', 'Registrazione non andata a buon fine.', 10000);
  //   this.notificationService.show('info', 'Registrazione non andata a buon fine', 10000);
  //   this.notificationService.show('warning', 'Registrazione non andata a buon fine', 10000);
  // }
}
