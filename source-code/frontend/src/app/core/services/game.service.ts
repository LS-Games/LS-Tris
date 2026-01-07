import { inject, Injectable, NgZone, signal } from "@angular/core";
import { WebsocketService } from "./websocket.service";
import { AuthService } from "./auth.service";
import { RoundService } from "./round.service";

//We report the backend message structure about this service

interface GameInfo {
    id_game : number,
    creator_nickname : string,
    owner_nickname : string,
    owner_current_streak :number,
    owner_max_streak : number,
    state : string,
    created_at : string
}

@Injectable({ providedIn: 'root'}) 
export class GameService {

    private readonly _ws = inject(WebsocketService);
    private readonly _auth = inject(AuthService);
    private readonly _round = inject(RoundService);
    
    gamesSignal = signal<GameInfo[]>([]);
    rematchPendingSignal = signal<boolean>(false);

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

        this._ws.onAction<any>('server_game_updated')
        .subscribe(msg => {
            console.log('[WS] game updated:', msg.game);
            const updatedGame = msg.game;

            this.gamesSignal.update(games =>
                games.map(g =>
                    g.id_game === updatedGame.id_game
                    ? { ...g, ...updatedGame }
                    : g
        )
    );
});

    }

    setGames(games: GameInfo[]) {
        this.gamesSignal.set(games);
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
        if(!this._round.gameId()) {
            console.warn("GameId is null!");
            return;
        }

        const id_game = this._round.gameId();

        const payload = {
            action: 'game_end',
            id_game,
            id_owner: this._auth.id
        };

        this._ws.send(payload)
    }

    rematchGame() {
        if(!this._round.gameId()) {
            console.warn("GameId is null!");
            return;
        }

        const id_game = this._round.gameId();

        const payload = {
            action: 'game_accept_rematch',
            id_game,
            id_player_accepting_rematch : this._auth.id
        }

        this._ws.send(payload);
    }

    refuseRematchGame() {
        if(!this._round.gameId()) {
            console.warn("GameId is null");
            return;
        }

        const id_game = this._round.gameId();

        const payload = {
            action: 'game_refuse_rematch',
            id_game
        }

        this._ws.send(payload);
    }

    forfeitGame() {
        const id_game = this._round.gameId();
        const id_player = this._auth.id;

        const payload = {
            action: 'game_forfeit',
            id_game : id_game,
            id_player : id_player
        }

        this._ws.send(payload);
    }
}