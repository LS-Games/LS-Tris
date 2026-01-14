import { Routes } from '@angular/router';
import { authGuard } from './core/guards/auth.guard';
import { LeaveGameGuard } from './core/guards/leave-game.guard';

export const routes: Routes = [
    {
        /*
            We use the loadComponent that is a property which Routes expects in the route,
            It must be a function that returns a Promise.
            We could use "Component" instead of "loadComponent" but the first isn't efficient,
            before Angular v15 all components of all routes have been load beforehand with "Component".
            The arrival of "loadComponent" has made things more efficient because it only loads component
            when they are needed (lazy load).
        */

        path: '', //Default route is homepage
        loadComponent: () => 
            import('./pages/homepage/home').then(m => m.Home),
    },
    {
        path: 'round/:gameId/:roundId', 
        canActivate: [authGuard],
        canDeactivate: [LeaveGameGuard],
        loadComponent: () =>
            import('./pages/round-page/round-page').then(m => m.RoundPage),
    }, 
    {
        path: 'profile/:playerId',
        canActivate: [authGuard],
        loadComponent: () => 
            import('./pages/profile-page/profile-page').then(m => m.ProfilePage)
    }
];
