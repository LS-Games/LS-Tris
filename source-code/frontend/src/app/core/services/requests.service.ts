import { inject, Injectable } from "@angular/core";
import { WebsocketService } from "./websocket.service"; 

@Injectable({providedIn: 'root'}) 
export class RequestsService {

    private readonly ws = inject(WebsocketService);

    requestParticipation( id_game:number, id_player:number) {
        const msg = {
            action: 'participation_request_send',
            id_game: id_game,
            id_player: id_player
        }
        this.ws.send(msg);
    }
}
