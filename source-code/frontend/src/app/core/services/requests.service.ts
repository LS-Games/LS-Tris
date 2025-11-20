import { inject, Injectable, signal } from "@angular/core";
import { WebsocketService } from "./websocket.service"; 

interface RequestInfo {
    id_request : number,
    id_game : number,
    player_nickname : string,
    state : string,
    created_at : string
}

@Injectable({providedIn: 'root'}) 
export class RequestsService {

    private readonly _pendingSignal = signal(false);
    pendingSignal = this._pendingSignal.asReadonly();

    private readonly _ws = inject(WebsocketService);

    requestsSignal = signal<RequestInfo[]>([]);

    constructor() {
        this._ws.onAction<any>('server_new_participation_request')
        .subscribe(msg => {
            const newRequest = msg.participation_requests[0];
            this.requestsSignal.update(requests => [...requests, newRequest]);
        });
    }

    requestParticipation( id_game:number, id_player:number) {
        const msg = {
            action: 'participation_request_send',
            id_game: id_game,
            id_player: id_player
        }
        this._ws.send(msg);
    }

    startPending() {
        this._pendingSignal.set(true);
    }

    endPending() {
        this._pendingSignal.set(false);
    }
}
