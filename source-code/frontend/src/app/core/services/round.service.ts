import { inject, Injectable, signal } from "@angular/core";
import { WebsocketService } from "./websocket.service";
import { AuthService } from "./auth.service";

@Injectable({ providedIn: 'root'})
export class RoundService {

    private readonly _ws = inject(WebsocketService);
    private readonly _auth = inject(AuthService);

    gameId = signal<number | null>(null);
    roundId = signal<number | null>(null);
    player1Id = signal<number | null>(null);
    player2Id = signal<number | null>(null);
    player1Nickname = signal<string | null>(null);
    player2Nickname = signal<string | null>(null);

    boardSignal = signal<(null | 'X' | 'O')[]>(Array(9).fill(null));
    currentPlayerTurnSignal = signal<'X' | 'O'>('X');
    lastMoveIndexSignal = signal<number | null>(null);

    mySymbolSignal = signal<'X' | 'O'>('X');
    winnerSignal = signal<string | null>(null);

    constructor() {

        this._ws.onAction<any>('server_round_start') 
            .subscribe(msg => {

                const round = msg.round; 
                this.gameId.set(round.id_game);
                this.roundId.set(round.id_round);
                this.player1Id.set(round.id_player1);
                this.player2Id.set(round.id_player2);
                this.player1Nickname.set(round.nickname_player1);
                this.player2Nickname.set(round.nickname_player2);

                if(this.player1Id() === this._auth.id) {
                    this.mySymbolSignal.set('X');
                } else if (this.player2Id() === this._auth.id) {
                    this.mySymbolSignal.set('O');
                }
            });

        this._ws.onAction<any>('round_make_move')
            .subscribe(msg => {
                if (msg.status === 'success') {

                    const index = this.lastMoveIndexSignal();
                    if (index !== null) {
                        this.boardSignal.update(b => {
                            const copy = [...b];
                            copy[index] = this.mySymbolSignal();
                            return copy;
                        });

                        this.lastMoveIndexSignal.set(null);
                    }
                }
            });


        this._ws.onAction<any>('server_updated_round_move')
            .subscribe(msg => {
                console.log(msg);
                this.updateBoard(msg.rounds[0].board);
            })

        this._ws.onAction<any>('server_updated_round_end')
            .subscribe(msg => {
                const round = msg.rounds[0];
                console.log("ROUND ENDED", round);

                this.roundId.set(round.id_round);
                this.updateBoard(round.board);

                this.winnerSignal.set(round.winner ?? null);
            });

        this._ws.onAction<any>('server_round_end_notification')
            .subscribe(msg => {
                console.log("NOTIFICATION: ROUND ENDED", msg);

                this.currentPlayerTurnSignal.set('X');
            });
    }

    updateBoard(board:string) {
        /**
         * With split() if we have "X@O@@X@O" it provides us ["X", "@", "O", "@", "@", "X", "@", "O"]
           Moreover if there are @ symbols these are repleced with null
         */
        const arr = board.split('').map(c => {
        if (c === '@') return null;
        return c as 'X' | 'O';
        });

        //Update the board
        this.boardSignal.set(arr);

        const turn = this.computeCurrentTurn(arr);
        this.currentPlayerTurnSignal.set(turn);
        console.log(`CurrentPlayerSignal ${this.currentPlayerTurnSignal()}`);
    }

    computeCurrentTurn(board:(null | 'X' | 'O')[]) {
        const xCount = board.filter(c => c === 'X').length;
        const oCount = board.filter(c => c === 'O').length;

        return xCount === oCount ? 'X' : 'O';
    }

    clear() {
        this.gameId.set(null);
        this.roundId.set(null);
        this.player1Id.set(null);
        this.player2Id.set(null);
        this.player1Nickname.set(null);
        this.player2Nickname.set(null);
    }

    makeMove(index:number) {

        console.log("ROUND ID →", this.roundId());
        console.log("GAME ID →", this.gameId());


        console.log(`MySimbolSignal: ${this.mySymbolSignal()} | currentPlayerSignal: ${this.currentPlayerTurnSignal()}`);
        if(this.mySymbolSignal() !== this.currentPlayerTurnSignal()) {
            console.warn("Non è il tuo turno");
        }

        if(this.boardSignal()[index] !== null) {
            console.warn("Cella già occupata");
        }

        this.lastMoveIndexSignal.set(index);

        const row = Math.floor(index/3);
        const col = index%3;
        console.log(`col: ${col}, row: ${row}`);

        const payload = {
            action: 'round_make_move',
            id_round: this.roundId(),
            id_player: this._auth.id,
            id_game: this.gameId(),
            row: row,
            col: col
        }
        
        console.log(payload);
        this._ws.send(payload);
    }

}