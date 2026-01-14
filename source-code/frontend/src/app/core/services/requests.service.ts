import { inject, Injectable, numberAttribute, signal } from "@angular/core";
import { WebsocketService } from "./websocket.service"; 
import { AuthService } from "./auth.service";

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

    activeGameId = signal<number | null>(null);
    pendingRequestId = signal<number | null>(null);

    private readonly _ws = inject(WebsocketService);
    private readonly _auth = inject(AuthService);

    requestsSignal = signal<RequestInfo[]>([]);

    constructor() {

        this._ws.onAction<any>('participation_request_send') 
        .subscribe( msg => {
            console.log(msg);
            if(msg.status === 'success') {
                this.pendingRequestId.set(msg.id);
                this._pendingSignal.set(true);
            }
        });

        this._ws.onAction<any>('server_new_participation_request')
        .subscribe(msg => {

            console.log(msg);
            const newRequest = msg.participation_requests[0];
            this.requestsSignal.update(requests => [...requests, newRequest]);
        });

        this._ws.onAction<any>('server_participation_request_cancel')
            .subscribe(msg => {
                console.log(msg);
                const idToRemove = Number(msg.id_request);

                this.requestsSignal.update(requests =>
                    requests.filter(r => {
                        return r.id_request !== idToRemove;
                    })
                );
        });

        this._ws.onAction<any>('participation_request_change_state')
            .subscribe(msg => {

                console.log(msg);

                if(msg.status === 'success') {
                    const idToRemove = Number(msg.id);

                    this.requestsSignal.update(requests =>
                        requests.filter(r => {
                            return r.id_request !== idToRemove;
                        })
                    );
                }
        });

        this._ws.onAction<any>('server_participation_request_change')
            .subscribe(msg => {

                console.log(msg);

                if(msg.status === 'success') {
                    this.endPending();
                }
        });

        this._ws.onAction<any>('server_active_game')
            .subscribe(msg => {
                console.log(msg);
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

    deleteMyParticipationRequest() {
        const payload = {
            action: 'participation_request_cancel',
            id_participation_request: this.pendingRequestId(),
            id_player: this._auth.id
        };

        this._ws.send(payload);
    }

    rejectParticipationRequest(id_request:number) {

        const payload = {
            action: 'participation_request_change_state',
            id_participation_request: id_request,
            new_state: 'rejected' 
        }

        this._ws.send(payload);
    }

    rejectAllParticipationRequests() {

        const payload = {
            action: 'participation_request_reject_all',
            requests: this.requestsSignal()
        }

        console.log(payload);

        this._ws.send(payload);
    }

    acceptParticipationReqeust(id_request:number) {

        const payload = {
            action: 'participation_request_change_state',
            new_state: 'accepted',
            id_participation_request: id_request,
        }

        this._ws.send(payload);
    }

    clearRequests() {
        this.requestsSignal.set([]);
    }
}
