import { Component, OnInit } from '@angular/core';
import { CommonModule } from '@angular/common';
import { NotificationService, Notification } from '../../core/services/notification';

@Component({
  selector: 'app-notifications',
  standalone: true,
  imports: [CommonModule],
  templateUrl: './notifications.html',
  styleUrl: './notifications.scss'
})
export class NotificationsComponent implements OnInit {

  notifications: Notification[] = [];

  constructor(private notificationService: NotificationService) {}

  ngOnInit(): void {
    //We signup to BehaviorSubject of the service
    //$ is a convention which indicates that it is a Observable
    this.notificationService.notifications$.subscribe(list => {
      this.notifications = list;
    });
  }

  remove(id: number): void {
    this.notificationService.remove(id);
  }
}
