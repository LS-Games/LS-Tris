import { inject, Injectable, NgZone } from "@angular/core";
import { WebsocketService } from "./websocket.service";
import { AuthService } from "./auth.service";
import { Observable, Subject } from "rxjs";

//We report the backend message structure about this service

interface GameInfo {
    id_game : string,
    creator_nickname : string,
    owner_nickname : string,
    status : string;
    created_at : string
}

interface BackendGamesListResponse {
    action: 'games_get_public_info';
    count?: number;
    status: 'success' | 'error';
    games: GameInfo[];
    error_message?: string;
}

interface BackendGameCreateResponse {
    action: 'game_start';
    status: 'success' | 'error';
    id?: number;
    error_message?: string;
}

interface BackendGameDeleteResponse {
    action: 'game_cancel';
    status: 'success' | 'error';
    id?: number;
    error_message ?: string;
}

@Injectable({ providedIn: 'root'}) 
export class GameService {

    private readonly _ws = inject(WebsocketService);
    private readonly _auth = inject(AuthService);
    // private readonly _zone = inject(NgZone);

    private readonly gameCreated$ = new Subject<number>();
    private readonly gameError$ = new Subject<string>();

    /**
     * In this case, unlike others, we only have to return the result to 
     * the calling function, in fact we dont'do .subscribe() because we don't need
     * to do operation on stream.
     * 
     * TIP: Use Subscribe() only in methods which have side effects (they have to do 
     * something internally), while for methods which provides data we have always 
     * return a pure Oberservable.
     */

    getAllGame() : Observable<BackendGamesListResponse> {
        const payload = { action: 'games_get_public_info', status: 'all' };
        this._ws.send(payload);
        return this._ws.onAction<BackendGamesListResponse>('games_get_public_info');
    }

    createGame() : void {

        //Send request to backend
        const payload = { action: 'game_start', id_creator: `${this._auth.id}`};
        this._ws.send(payload);

        //Listen only to "game_start" responses

        this._ws.onAction<BackendGameCreateResponse>('game_start')
            .subscribe((backend) => {
                if(backend.status === 'success' && backend.id) {
                    this.gameCreated$.next(backend.id);
                } else {
                    this.gameError$.next(backend.error_message || 'Unknown error');
                }
            });
    }

    deleteGame(id_game : number) : void {

        const payload = { action: 'game_cancel', id_game: `${id_game}`};
        this._ws.send(payload);

        this._ws.onAction<BackendGameDeleteResponse>('game_cancel')
            .subscribe((backend) => {
                if(backend.status === 'success' && backend.id) {
                    this.gameCreated$.next(backend.id);
                } else {
                    this.gameError$.next(backend.error_message || 'Unknown error');
                }
            });
    }

    onGameCreated() {
        return this.gameCreated$.asObservable();
    }

    onGameError() {
        return this.gameError$.asObservable();
    }
}


