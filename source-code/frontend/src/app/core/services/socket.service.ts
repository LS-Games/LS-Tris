//Injectable decorator is using to define a service that can be injected into other components
//or services via dependency injection
import { Injectable } from '@angular/core';

//Whit 'root' Angular creates only one instance of SocketService for the whole application 
//Which is directly injected into the root 
@Injectable({ providedIn: 'root'})
export class SocketService {

    //WebSocket type is provided by native browser class, WebSocket, which allows for a bidirectional connection to the server
    private _socket: WebSocket;

    constructor() {
        //ws:// is a protocol similar to https:// but with persistence
        //We pass him the listening bridge port
        this._socket = new WebSocket('ws://localhost:3001')
    }

    //Send takes object (JSON object) as a parameter
    //The stringfy() method converts the object into JSON text because it only sends strings.
    send(data: any) {
        this._socket.send(JSON.stringify(data));
    }

    //This method is using to listen the messages that arrive on the server
    //callback is a parameter with function type
    onMessage(callback: (data: any) => void) {

        //'message' is the event type that activates itself 
        //event is a parameter of the anonymus function

        this._socket.addEventListener('message', event => {

            //JSON parse converts the string into a JavaScript object
            const parsed = JSON.parse(event.data);

            //callback function is called and the parsed JSON is passed as a parameter 
            //Note that the function is only called if the event has been activated
            callback(parsed);
        });
    }

}
