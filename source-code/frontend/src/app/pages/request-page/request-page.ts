import { afterNextRender, Component, inject, DestroyRef } from '@angular/core';
import { DialogRef } from '@angular/cdk/dialog';
import { GameService } from '../../core/services/game.service';
import { takeUntilDestroyed } from '@angular/core/rxjs-interop';


@Component({
  selector: 'app-request-page',
  standalone: true,
  imports: [],
  templateUrl: './request-page.html',
  styleUrl: './request-page.scss'
})

export class RequestPage {

  private readonly _dialogRef = inject(DialogRef<RequestPage>);

  //DestroyRedf is a object that Angular provides to manage a life cycle of a component
  private readonly _destroyRef = inject(DestroyRef);
  private readonly _game = inject(GameService);

  loading = true;
  id_game: number | null = null;
  error_message: string | null = null;

  constructor() {
    afterNextRender(() => {

      this._game.createGame();

      /**
       * pipe() is a RxJS method to modify/filter the flow (we can use RxJs operatore before that they arrive to subscribe)
       * takeUntilDestroyer() creates a trigger automatically which is connected to component destruction.
       * With Subscribe() we subscribe ourself to the stream and every time that a new ID is issued, save that ID in the component variable
       */

      this._game.onGameCreated().pipe(takeUntilDestroyed(this._destroyRef)).subscribe((id) => {
        this.id_game = id;
        this.loading = false;
        console.log('Game created succesfully with id: ', id);

      });

      this._game.onGameError().pipe(takeUntilDestroyed(this._destroyRef)).subscribe((err) => {
        this.error_message = err;
        this.loading = false;
        console.log("Error about game creation");
        this.close();
      });
    });
  }

  close() {
    this._dialogRef.close();

    if(this.id_game) {
      this._game.deleteGame(this.id_game);
    }
  }
}
