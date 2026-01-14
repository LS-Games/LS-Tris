import { Component, effect, inject} from '@angular/core';
import { RouterOutlet } from '@angular/router';
import { Header } from './layout/header/header';
import { Footer } from './layout/footer/footer';
import { NotificationsComponent } from './features/notifications/notifications';
import { NotificationService } from './core/services/notification';
import { WebsocketService } from './core/services/websocket.service';

@Component({
  selector: 'app-root',
  standalone: true,
  imports: [RouterOutlet, Header, Footer, NotificationsComponent],
  templateUrl: './app.html',
  styleUrls: ['./app.scss']
})

export class App {

  private readonly _ws = inject(WebsocketService);
  private readonly notifications = inject(NotificationService);

  constructor() {
    effect(() => {
      this._ws.connect();

      this._ws.onAction('server_new_game').subscribe( msg => {
        this.notifications.show('info', `New game created by ${msg.games[0].owner_nickname}`, 6000);
      })
    })
  }

}
