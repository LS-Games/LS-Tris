// leave-game.guard.ts
import { Injectable } from '@angular/core';
import { CanDeactivate } from '@angular/router';
import { CanComponentDeactivate } from './can-component-deactivate'

/**
 * Asks the active component whether route deactivation is allowed.
 * Defaults to allowing navigation if no explicit check is provided.
 */

@Injectable({ providedIn: 'root' })
export class LeaveGameGuard
  implements CanDeactivate<CanComponentDeactivate> {

  canDeactivate(component: CanComponentDeactivate) {

    /**
     * The guard does not know anything about the game state.
     * Its only responsibility is to delegate the decision
     * to the active component, asking whether it is safe
     * to leave the current view.
     */

    return component.canDeactivate ? component.canDeactivate() : true;
  }
}
