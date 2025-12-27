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
    rematchPendingSignal = signal<boolean>(false);

    boardSignal = signal<(null | 'X' | 'O')[]>(Array(9).fill(null));
    currentPlayerTurnSignal = signal<'X' | 'O'>('X');
    lastMoveIndexSignal = signal<number | null>(null);

    mySymbolSignal = signal<'X' | 'O'>('X');
    winnerSignal = signal<'X' | 'O' | null>(null);
    roundEndedSignal = signal<boolean>(false);

    constructor() {

        this._ws.onAction<any>('server_round_start') 
            .subscribe(msg => {

                console.log(msg);
                this.resetRoundState();

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


        this._ws.onAction<any>('server_updated_round_move')
            .subscribe(msg => {
                console.log(msg);
                this.updateBoard(msg.rounds[0].board);
                this.lastMoveIndexSignal.set(null);
            })

        this._ws.onAction<any>('server_updated_round_end')
            .subscribe(msg => {
                console.log(msg);
                const round = msg.rounds[0];
                this.roundId.set(round.id_round);
                this.updateBoard(round.board);

                if(this.findWinner(round.board)) {

                    this.winnerSignal.set(this.findWinner(round.board));
                    if(this.winnerSignal() === this.mySymbolSignal()) {
                        this.roundEndedSignal.set(true);
                        console.log("Hai vinto");
                    } else {
                        this.roundEndedSignal.set(true);
                        console.log("Hai perso");
                    }

                } else {
                    this.roundEndedSignal.set(true);
                    console.log("Pareggio");
                }
                
            });

        this._ws.onAction<any>('server_round_end_notification')
            .subscribe(msg => {
                console.log(msg);

                this.currentPlayerTurnSignal.set('X');
            });

        this._ws.onAction<any>('game_accept_rematch')
            .subscribe(msg => {
                console.log(msg);

                if(msg.status === 'success' && msg.waiting === 1) {
                    this.rematchPendingSignal.set(true);
                }
            })
    }

    updateBoard(board:string) {

        /*
         * With split() if we have "X@O@@X@O" it provides us ["X", "@", "O", "@", "@", "X", "@", "O"]
         * Moreover if there are @ symbols these are repleced with null
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

    findWinner(board: string): 'X' | 'O' | null {

        const wins = [
            [0, 1, 2], 
            [3, 4, 5], 
            [6, 7, 8], 
            [0, 3, 6], 
            [1, 4, 7], 
            [2, 5, 8], 
            [0, 4, 8], 
            [2, 4, 6]  
        ];

        for (const [a, b, c] of wins) {

            const v1 = board[a];
            const v2 = board[b];
            const v3 = board[c];

            // Check that they're equal AND not '@'
            if (v1 !== '@' && v1 === v2 && v2 === v3) {
            return v1 as 'X' | 'O';
            }
        }

        return null;
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

    // --- RESET "UI / ROUND STATE" ---
    private resetRoundState() {
        this.boardSignal.set(Array(9).fill(null));
        this.currentPlayerTurnSignal.set('X');
        this.lastMoveIndexSignal.set(null);
        this.winnerSignal.set(null);
        this.roundEndedSignal.set(false); 
        this.rematchPendingSignal.set(false);
    }

    // --- RESET "SESSION / IDENTIFIERS" ---
    private resetRoundSession() {
        this.gameId.set(null);
        this.roundId.set(null);

        this.player1Id.set(null);
        this.player2Id.set(null);
        this.player1Nickname.set(null);
        this.player2Nickname.set(null);

        this.mySymbolSignal.set('X'); 
    }

    resetAll() {
        this.resetRoundSession();
        this.resetRoundState();
    }

    endPending() {
        this.rematchPendingSignal.set(false);
    }

}