import { inject, Injectable, NgZone, signal } from "@angular/core";
import { WebsocketService } from "./websocket.service";
import { AuthService } from "./auth.service";

//We report the backend message structure about this service

interface GameInfo {
    id_game : number,
    creator_nickname : string,
    owner_nickname : string,
    state : string,
    created_at : string
}

@Injectable({ providedIn: 'root'}) 
export class GameService {

    private readonly _ws = inject(WebsocketService);
    private readonly _auth = inject(AuthService);
    
    gamesSignal = signal<GameInfo[]>([]);
    currentGameIdSignal = signal<number | null>(null);

    constructor() {

        this._ws.onAction<any>('server_new_game')
        .subscribe(msg => {
            console.log(msg);
            const newGame = msg.games[0];
            this.gamesSignal.update(games => [...games, newGame]);
        });

        this._ws.onAction<any>('server_game_cancel')
        .subscribe(msg => {
            console.log(msg);
            const idToRemove = msg.id_game;
            this.gamesSignal.update(games =>
            games.filter(g => g.id_game !== idToRemove)
            );
        });
    }

    setGames(games: GameInfo[]) {
        this.gamesSignal.set(games);
    }

    setCurrentGameId(id:number | null) {
        this.currentGameIdSignal.set(id);
    }

    getAllGame() {
        const payload = { action: 'games_get_public_info', status: 'all' };
        this._ws.send(payload);
        return this._ws.onAction<any>('games_get_public_info');
    }

    createGame() {
        const payload = { action: 'game_start', id_creator: this._auth.id };
        this._ws.send(payload);
    }

    deleteGame(id_game: number) {
        const payload = { 
            action: 'game_cancel', 
            id_game, 
            id_owner: this._auth.id 
        };

        this._ws.send(payload);
    }

    endGame() {
        const id_game = this.currentGameIdSignal();
        if (!id_game) return;

        const payload = {
            action: 'game_end',
            id_game,
            id_owner: this._auth.id
        };

        this._ws.send(payload)
    }
}