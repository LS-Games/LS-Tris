import { Injectable } from '@angular/core';
import { BehaviorSubject } from 'rxjs';

export interface Notification {
  id: number;
  type: 'success' | 'error' | 'info' | 'warning';
  message: string;
  duration?: number; 
}

@Injectable({ providedIn: 'root'})

export class NotificationService {

  //Array is useful to save notifications
  private notifications: Notification[] = [];

  //Reactive object that notifies us when it changes
  private readonly notificationsSubject = new BehaviorSubject<Notification[]>([]);

  //We use asObservable() because we want that the components watch only 
  //Only the service can changes data
  public notifications$ = this.notificationsSubject.asObservable();
  private idCounter = 0;

  show(type: Notification['type'], message: string, duration = 3000) {

    //When this function is called, we create a new noitification object 
    const notification: Notification = {
      id: ++this.idCounter,
      type,
      message,
      duration
    };

    //We insert it in the array
    this.notifications.push(notification);

    //This says to Behavior Subject that it has a new value and says to send it to all subscribes
    this.notificationsSubject.next(this.notifications);

    if (duration > 0) {
      setTimeout(() => this.remove(notification.id), duration);
    }
  }

  remove(id: number) {
    this.notifications = this.notifications.filter(n => n.id !== id);
    this.notificationsSubject.next(this.notifications);
  }

  clearAll() {
    this.notifications = [];
    this.notificationsSubject.next([]);
  }
}
