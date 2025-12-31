/**
 * Contract used by route guards to determine whether
 * a component allows navigation away from its current view.
 *
 * Components implementing this interface must explicitly decide
 * if they can be deactivated (e.g. when a game is still in progress),
 * typically by showing a confirmation dialog or performing
 * cleanup / server notifications.
 *
 * This keeps navigation logic generic and delegates
 * state-specific decisions to the component itself.
 */

export interface CanComponentDeactivate {
  canDeactivate: () => boolean | Promise<boolean>;
}
