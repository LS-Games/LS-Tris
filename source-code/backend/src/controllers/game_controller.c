#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <inttypes.h>

#include "../../include/debug_log.h"

#include "game_controller.h"
#include "round_controller.h"
#include "player_controller.h"
#include "play_controller.h"
#include "notification_controller.h"
#include "../json-parser/json-parser.h"
#include "../server/server.h"
#include "../dao/sqlite/db_connection_sqlite.h"
#include "../dao/sqlite/game_dao_sqlite.h"

typedef struct {
    int64_t id_game;
    int64_t requested_by;
} PendingRematch;

static PendingRematch *g_pending_rematches = NULL;
static int g_pending_count = 0;
static pthread_mutex_t g_pending_mtx = PTHREAD_MUTEX_INITIALIZER;

/**
 * We check if there are pending rematch request for a given id_game
 * return index if found
 * return -1 if not found
 */
static int pending_find_index(int64_t id_game) {
    for (int i = 0; i < g_pending_count; i++) {
        if (g_pending_rematches[i].id_game == id_game) return i;
    }
    return -1;
}


/**
 * Set/update the rematch request
 * if id_game exists then it updates requested_by only 
 * if id_game doesn't exist then it extends the array and add the element at the bottom
 */
static int pending_set(int64_t id_game, int64_t requested_by) {
    int idx = pending_find_index(id_game);


    if (idx >= 0) {
        g_pending_rematches[idx].requested_by = requested_by;
        return 1;
    }

    PendingRematch *tmp = realloc(g_pending_rematches, sizeof(PendingRematch) * (g_pending_count + 1));
    if (!tmp) return 0;

    g_pending_rematches = tmp;
    g_pending_rematches[g_pending_count].id_game = id_game;
    g_pending_rematches[g_pending_count].requested_by = requested_by;
    g_pending_count++;
    return 1;
}

static void pending_clear(int64_t id_game) {
    int idx = pending_find_index(id_game);
    if (idx < 0) return;

    g_pending_rematches[idx] = g_pending_rematches[g_pending_count - 1];
    g_pending_count--;

    if (g_pending_count == 0) {
        free(g_pending_rematches);
        g_pending_rematches = NULL;
    }
}

// This function provides a query by `status`. 
// @param status Possible values are `new`, `active`, `waiting`, `finished` and `all` (no filter)
GameControllerStatus games_get_public_info(char *status, GameDTO **out_dtos, int *out_count) {

    LOG_DEBUG("Status: %s\n", status);

    GameStatus queryStatus = GAME_STATUS_INVALID;
    if (strcmp(status, "all") != 0) {
        queryStatus = string_to_game_status(status);
        if (queryStatus == GAME_STATUS_INVALID)
            return GAME_CONTROLLER_INVALID_INPUT;
    }

    GameWithPlayerNickname* retrievedGamesWithPlayerInfo;
    int retrievedObjectCount;
    if (game_find_all_with_player_info(&retrievedGamesWithPlayerInfo, &retrievedObjectCount) == GAME_CONTROLLER_NOT_FOUND) {
        *out_dtos = NULL;
        *out_count = 0;
        return GAME_CONTROLLER_NOT_FOUND;
    }

    GameDTO *dynamicDTOs = NULL;
    int filteredObjectCount = 0;
    for (int i = 0; i < retrievedObjectCount; i++) {
        if (strcmp(status, "all") == 0 || retrievedGamesWithPlayerInfo[i].state == queryStatus) {

            Game game = {
                .id_game = retrievedGamesWithPlayerInfo[i].id_game,
                .state = retrievedGamesWithPlayerInfo[i].state,
                .created_at = retrievedGamesWithPlayerInfo[i].created_at
            };

            dynamicDTOs = realloc(dynamicDTOs, (filteredObjectCount + 1) * sizeof(GameDTO));
            if (dynamicDTOs == NULL) {
                LOG_WARN("%s\n", "Memory not allocated");
                return GAME_CONTROLLER_INTERNAL_ERROR;
            }

            map_game_to_dto(&game, retrievedGamesWithPlayerInfo[i].creator, retrievedGamesWithPlayerInfo[i].owner, &(dynamicDTOs[filteredObjectCount]));

            filteredObjectCount = filteredObjectCount + 1;
        } 
    }

    *out_dtos = dynamicDTOs;
    *out_count = filteredObjectCount;
    
    return GAME_CONTROLLER_OK;
}

GameControllerStatus game_start(int64_t id_creator, int64_t* out_id_game) {

    // Build game to start
    Game gameToStart = {
        .id_creator = id_creator,
        .id_owner = id_creator,
        .created_at = time(NULL),
        .state = NEW_GAME
    };

    // Create game
    GameControllerStatus status = game_create(&gameToStart);
    if (status != GAME_CONTROLLER_OK)
        return status;

    // Send notification
    NotificationDTO *out_notification_dto = NULL;
    if (notification_new_game(gameToStart.id_game, id_creator, &out_notification_dto) != NOTIFICATION_CONTROLLER_OK)
        return GAME_CONTROLLER_INTERNAL_ERROR;
    char *json_message = serialize_notification_to_json("server_game_start_notification", out_notification_dto);
    if (send_server_broadcast_message(json_message, id_creator) < 0 ) {
        return GAME_CONTROLLER_INTERNAL_ERROR;
    }
    free(json_message);
    free(out_notification_dto);

    // Send updated game
    GameDTO out_game_dto;
    GameWithPlayerNickname retrievedGameWithPlayerNickname; // Retrieve players nicknames
    status = game_find_one_with_player_info(gameToStart.id_game, &retrievedGameWithPlayerNickname);
    if (status != GAME_CONTROLLER_OK) {
        return status;
    }
    map_game_to_dto(&gameToStart, retrievedGameWithPlayerNickname.creator, retrievedGameWithPlayerNickname.owner, &out_game_dto);
    json_message = serialize_games_to_json("server_new_game", &out_game_dto, 1);
    if (send_server_broadcast_message(json_message, gameToStart.id_owner) < 0 ) {
        return GAME_CONTROLLER_INTERNAL_ERROR;
    }
    free(json_message);

    *out_id_game = gameToStart.id_game;

    return GAME_CONTROLLER_OK;
}

GameControllerStatus game_end(int64_t id_game, int64_t id_owner, int64_t* out_id_game) {

    // Retrieve game to end
    Game retrievedGame;
    GameControllerStatus status = game_find_one(id_game, &retrievedGame);
    if (status != GAME_CONTROLLER_OK)
        return status;

    if (retrievedGame.id_owner != id_owner)
        return GAME_CONTROLLER_FORBIDDEN;

    retrievedGame.state = FINISHED_GAME;

    status = game_update(&retrievedGame);
    if (status != GAME_CONTROLLER_OK)
        return status;

    // Send updated game
    GameDTO out_game_dto;
    GameWithPlayerNickname retrievedGameWithPlayerNickname; // Retrieve players nicknames
    status = game_find_one_with_player_info(retrievedGame.id_game, &retrievedGameWithPlayerNickname);
    if (status != GAME_CONTROLLER_OK) {
        return status;
    }
    map_game_to_dto(&retrievedGame, retrievedGameWithPlayerNickname.creator, retrievedGameWithPlayerNickname.owner, &out_game_dto);
    char *json_message = serialize_games_to_json("server_end_game", &out_game_dto, 1);
    if (send_server_broadcast_message(json_message, retrievedGame.id_owner) < 0 ) {
        return GAME_CONTROLLER_INTERNAL_ERROR;
    }
    free(json_message);

    *out_id_game = retrievedGame.id_game;

    return GAME_CONTROLLER_OK;
}


/**
 * Finalizes a game due to player forfeit.
 *
 * This function is called when a player voluntarily leaves a game
 * (home navigation, logout, browser close, refresh, etc.).
 *
 * The winner is determined from the ACTIVE round and its Play entries:
 * the player who did NOT leave automatically wins.
 *
 * The function is idempotent:
 * - if the game is already finished, it safely returns
 * - this prevents duplicated events or race conditions
 *
 * Updates performed:
 * - Play.result  -> WIN / LOSE
 * - Round.state  -> FINISHED
 * - Game.state   -> FINISHED
 * - Game.id_owner -> winner
 *
 * @param id_game   Game identifier
 * @param id_leaver Player who left the game
 * @param out_winner Optional output winner id (can be NULL)
 *
 * @return GAME_CONTROLLER_OK on success, or an error code
 */

GameControllerStatus game_forfeit(int64_t id_game, int64_t id_leaver, int64_t* out_winner) {
    Game game;
    GameControllerStatus gstatus = game_find_one(id_game, &game);
    if (gstatus != GAME_CONTROLLER_OK)
        return gstatus;

    /* Idempotency guard: if already finished, do nothing */
    if (game.state == FINISHED_GAME)
        return GAME_CONTROLLER_OK;

    /* 1. Find ACTIVE round for this game                  */
    Round *rounds = NULL;
    int round_count = 0;

    RoundControllerStatus rstatus = round_find_all(&rounds, &round_count);
    if (rstatus != ROUND_CONTROLLER_OK && rstatus != ROUND_CONTROLLER_NOT_FOUND)
        return GAME_CONTROLLER_INTERNAL_ERROR;

    Round *selected_round = NULL;

    /* 1. Try to find an ACTIVE round */
    for (int i = 0; i < round_count; i++) {
        if (rounds[i].id_game == id_game &&
            rounds[i].state == ACTIVE_ROUND) {
            selected_round = &rounds[i];
            break;
        }
    }

    /* 2. Fallback: use the last round of the game (if no ACTIVE one exists) */
    if (!selected_round) {
        for (int i = 0; i < round_count; i++) {
            if (rounds[i].id_game == id_game) {
                selected_round = &rounds[i];
            }
        }
    }

    /* 3. No rounds at all → real NOT_FOUND */
    if (!selected_round) {
        free(rounds);
        return GAME_CONTROLLER_NOT_FOUND;
    }

    /* 2. Retrieve Play entries for the active round       */
    Play *plays = NULL;
    int play_count = 0;

    PlayControllerStatus pstatus =
        play_find_all_by_id_round(&plays, selected_round->id_round, &play_count);

    if (pstatus != PLAY_CONTROLLER_OK || play_count != 2) {
        free(rounds);
        return GAME_CONTROLLER_INTERNAL_ERROR;
    }

    /* 3. Determine winner (the one who did NOT leave)     */
    int64_t winner = -1;

    for (int i = 0; i < play_count; i++) {
        if (plays[i].id_player != id_leaver) {
            winner = plays[i].id_player;
            break;
        }
    }

    if (winner < 0) {
        free(plays);
        free(rounds);
        return GAME_CONTROLLER_FORBIDDEN;
    }

    /* 4. Update Play results (WIN / LOSE)                 */
    for (int i = 0; i < play_count; i++) {
        if (plays[i].id_player == winner) {
            plays[i].result = WIN;
        } else {
            plays[i].result = LOSE;
        }

        if (play_update(&plays[i]) != PLAY_CONTROLLER_OK) {
            free(plays);
            free(rounds);
            return GAME_CONTROLLER_DATABASE_ERROR;
        }
    }
                              
    /* 5. Finalize Round (only if still active) */
    if (selected_round->state == ACTIVE_ROUND) {

        selected_round->state = FINISHED_ROUND;

        if (round_update(selected_round) != ROUND_CONTROLLER_OK) {
            free(plays);
            free(rounds);
            return GAME_CONTROLLER_DATABASE_ERROR;
        }
    }

    /* 6. Finalize Game                                    */
    game.id_owner = winner;
    game.state = WAITING_GAME;

    gstatus = game_update(&game);
    if (gstatus != GAME_CONTROLLER_OK) {
        free(plays);
        free(rounds);
        return gstatus;
    }

    if (out_winner)
        *out_winner = winner;

    free(plays);
    free(rounds);
    return GAME_CONTROLLER_OK;
}

GameControllerStatus game_refuse_rematch(int64_t id_game, int64_t* out_id_game) {

    Game retrievedGame;
    GameControllerStatus status = game_find_one(id_game, &retrievedGame);
    if (status != GAME_CONTROLLER_OK)
        return status;

    //Clear any pending rematch request in RAM
    pthread_mutex_lock(&g_pending_mtx);
    pending_clear(id_game);
    pthread_mutex_unlock(&g_pending_mtx);

    // Update game state in order to let anyone send participation requests
    retrievedGame.state = WAITING_GAME;

    status = game_update(&retrievedGame);
    if (status != GAME_CONTROLLER_OK)
        return status;

    // Send notification
    NotificationDTO *out_notification_dto = NULL;
    if (notification_waiting_game(retrievedGame.id_game, retrievedGame.id_owner, &out_notification_dto) != NOTIFICATION_CONTROLLER_OK)
        return GAME_CONTROLLER_INTERNAL_ERROR;
    char *json_message = serialize_notification_to_json("server_game_waiting_notification", out_notification_dto);
    if (send_server_broadcast_message(json_message, retrievedGame.id_owner) < 0 ) {
        return GAME_CONTROLLER_INTERNAL_ERROR;
    }
    free(json_message);
    free(out_notification_dto);

    // Send updated game
    GameDTO out_game_dto;
    GameWithPlayerNickname retrievedGameWithPlayerNickname; // Retrieve players nicknames
    status = game_find_one_with_player_info(retrievedGame.id_game, &retrievedGameWithPlayerNickname);
    if (status != GAME_CONTROLLER_OK) {
        return status;
    }
    map_game_to_dto(&retrievedGame, retrievedGameWithPlayerNickname.creator, retrievedGameWithPlayerNickname.owner, &out_game_dto);
    json_message = serialize_games_to_json("server_waiting_game", &out_game_dto, 1);
    if (send_server_broadcast_message(json_message, retrievedGame.id_owner) < 0 ) {
        return GAME_CONTROLLER_INTERNAL_ERROR;
    }
    free(json_message);

    *out_id_game = retrievedGame.id_game;

    return GAME_CONTROLLER_OK;
}

/**
 * Handles a rematch request using a simple in-memory handshake.
 *
 * The first player requesting a rematch is put in a waiting state.
 * When the other player also requests a rematch, a new round is created.
 *
 * Active rounds are checked to prevent duplicate rematch creation.
 * No database schema or persistent state is modified.
 *
 * @param id_game Game identifier.
 * @param id_playerAcceptingRematch Player requesting or accepting the rematch.
 * @param out_id_game Output game identifier.
 * @param out_waiting 1 if waiting for the opponent, 0 if the round starts.
 *
 * @return GAME_CONTROLLER_OK on success, or an error code on failure.
 */

GameControllerStatus game_accept_rematch(int64_t id_game, int64_t id_playerAcceptingRematch, int64_t* out_id_game, int* out_waiting) {

    /* Default behavior: not waiting */
    if (out_waiting) *out_waiting = 0;

    /* Retrieve game information */
    Game retrievedGame;
    GameControllerStatus gameStatus = game_find_one(id_game, &retrievedGame);
    if (gameStatus != GAME_CONTROLLER_OK)
        return gameStatus;

    int64_t new_round_id;

    /* Step 1: Prevent duplicate ACTIVE rounds for the same game */
    Round *rounds = NULL;
    int count = 0;

    RoundControllerStatus roundStatus1 = round_find_all(&rounds, &count);
    if (roundStatus1 != ROUND_CONTROLLER_OK && roundStatus1 != ROUND_CONTROLLER_NOT_FOUND)
        return GAME_CONTROLLER_INTERNAL_ERROR;

    for (int i = 0; i < count; i++) {
        if (rounds[i].id_game == id_game && rounds[i].state == ACTIVE_ROUND) {
            LOG_INFO("An ACTIVE round already exists, avoiding duplicate creation");
            free(rounds);
            *out_id_game = id_game;
            if (out_waiting) *out_waiting = 0;
            return GAME_CONTROLLER_OK;
        }
    }

    if (rounds) free(rounds);

    /* Step 2: In-memory handshake for rematch */
    pthread_mutex_lock(&g_pending_mtx);

    int idx = pending_find_index(id_game);

    /* First player requests rematch */
    if (idx < 0) {
        if (!pending_set(id_game, id_playerAcceptingRematch)) {
            pthread_mutex_unlock(&g_pending_mtx);
            return GAME_CONTROLLER_INTERNAL_ERROR;
        }

        pthread_mutex_unlock(&g_pending_mtx);

        LOG_INFO("Rematch requested by player %" PRId64 ", waiting for opponent", id_playerAcceptingRematch);

        *out_id_game = id_game;
        if (out_waiting) *out_waiting = 1;
        return GAME_CONTROLLER_OK;
    }

    int64_t firstRequester = g_pending_rematches[idx].requested_by;

    /* Same player clicked rematch again */
    if (firstRequester == id_playerAcceptingRematch) {
        pthread_mutex_unlock(&g_pending_mtx);

        LOG_INFO("Player %" PRId64 " already requested rematch, still waiting", id_playerAcceptingRematch);

        *out_id_game = id_game;
        if (out_waiting) *out_waiting = 1;
        return GAME_CONTROLLER_OK;
    }

    /* Second player confirms rematch: clear pending and proceed */
    pending_clear(id_game);
    pthread_mutex_unlock(&g_pending_mtx);

    /* Step 3: Start a new round with the two rematch players (firstRequester vs second clicker) */
    RoundControllerStatus roundStatus2 = round_start(retrievedGame.id_game, firstRequester, id_playerAcceptingRematch, 500, &new_round_id);
    if (roundStatus2 != ROUND_CONTROLLER_OK) {
        LOG_WARN("%s", return_round_controller_status_to_string(roundStatus2));
        return GAME_CONTROLLER_INTERNAL_ERROR;
    }

    /* Build and send server_round_start event to both players */
    RoundFullDTO out_full_round;
    if (round_find_full_info_by_id_round(new_round_id, &out_full_round) != ROUND_CONTROLLER_OK)
        return GAME_CONTROLLER_INTERNAL_ERROR;

    char *json_new_round_for_rematch = serialize_round_full_to_json("server_round_start", &out_full_round);
    if (!json_new_round_for_rematch)
        return GAME_CONTROLLER_INTERNAL_ERROR;

    int64_t id_player1 = firstRequester;
    int64_t id_player2 = id_playerAcceptingRematch;

    LOG_INFO("Sending server_round_start to players %" PRId64 " and %" PRId64, id_player1, id_player2);

    /* Best-effort delivery: do not fail the rematch if a websocket send fails */
    if (send_server_unicast_message(json_new_round_for_rematch, id_player1) < 0)
        LOG_WARN("Failed to unicast server_round_start to player %" PRId64, id_player1);

    if (send_server_unicast_message(json_new_round_for_rematch, id_player2) < 0)
        LOG_WARN("Failed to unicast server_round_start to player %" PRId64, id_player2);

    free(json_new_round_for_rematch);

    LOG_INFO("Rematch accepted, new round started with id_round=%" PRId64, new_round_id);

    *out_id_game = retrievedGame.id_game;
    if (out_waiting) *out_waiting = 0;

    return GAME_CONTROLLER_OK;
}

GameControllerStatus game_cancel(int64_t id_game, int64_t id_owner, int64_t* out_id_game) {

    NotificationDTO *out_notification_dto = NULL;
    if(notification_game_cancel(id_game, id_owner, &out_notification_dto) != NOTIFICATION_CONTROLLER_OK)
        return GAME_CONTROLLER_INTERNAL_ERROR;
    char *json_message = serialize_notification_to_json("server_game_cancel", out_notification_dto);
    if (send_server_broadcast_message(json_message, id_owner) < 0 ) {
        free(json_message);
        free(out_notification_dto);
        return GAME_CONTROLLER_INTERNAL_ERROR;
    }

    free(json_message);
    free(out_notification_dto);

    GameControllerStatus status = game_delete(id_game);

    if (status != GAME_CONTROLLER_OK)
        return status;

    *out_id_game = id_game;

    return GAME_CONTROLLER_OK;
}

// ===================== Controllers Helper Functions =====================

GameControllerStatus game_change_owner(int64_t id_game, int64_t id_newOwner) {

    Game retrievedGame;
    GameControllerStatus status = game_find_one(id_game, &retrievedGame);
    if (status != GAME_CONTROLLER_OK)
        return status;

    retrievedGame.id_owner = id_newOwner;


    return game_update(&retrievedGame);
}

// ===================== CRUD Operations =====================

const char *return_game_controller_status_to_string(GameControllerStatus status) {
    switch (status) {
        case GAME_CONTROLLER_OK:               return "GAME_CONTROLLER_OK";
        case GAME_CONTROLLER_INVALID_INPUT:    return "GAME_CONTROLLER_INVALID_INPUT";
        case GAME_CONTROLLER_NOT_FOUND:        return "GAME_CONTROLLER_NOT_FOUND";
        // case GAME_CONTROLLER_STATE_VIOLATION:  return "GAME_CONTROLLER_STATE_VIOLATION";
        case GAME_CONTROLLER_DATABASE_ERROR:   return "GAME_CONTROLLER_DATABASE_ERROR";
        // case GAME_CONTROLLER_CONFLICT:         return "GAME_CONTROLLER_CONFLICT";
        case GAME_CONTROLLER_FORBIDDEN:        return "GAME_CONTROLLER_FORBIDDEN";
        case GAME_CONTROLLER_INTERNAL_ERROR:   return "GAME_CONTROLLER_INTERNAL_ERROR";
        default:                                return "GAME_CONTROLLER_UNKNOWN";
    }
}

// Create
GameControllerStatus game_create(Game* gameToCreate) {
    sqlite3* db = db_open();
    GameDaoStatus status = insert_game(db, gameToCreate);
    db_close(db);
    if (status != GAME_DAO_OK) {
        LOG_WARN("%s\n", return_game_dao_status_to_string(status));
        return GAME_CONTROLLER_DATABASE_ERROR;
    }

    return GAME_CONTROLLER_OK;
}

// Read all
GameControllerStatus game_find_all(Game **retrievedGameArray, int* retrievedObjectCount) {
    sqlite3* db = db_open();
    GameDaoStatus status = get_all_games(db, retrievedGameArray, retrievedObjectCount);
    db_close(db);
    if (status != GAME_DAO_OK) {
        LOG_WARN("%s\n", return_game_dao_status_to_string(status));
        return GAME_CONTROLLER_DATABASE_ERROR;
    }

    return GAME_CONTROLLER_OK;
}

// Read one
GameControllerStatus game_find_one(int64_t id_game, Game* retrievedGame) {
    sqlite3* db = db_open();
    GameDaoStatus status = get_game_by_id(db, id_game, retrievedGame);
    db_close(db);
    if (status != GAME_DAO_OK) {
        LOG_WARN("%s\n", return_game_dao_status_to_string(status));
        return status == GAME_DAO_NOT_FOUND ? GAME_CONTROLLER_NOT_FOUND : GAME_CONTROLLER_DATABASE_ERROR;
    }

    return GAME_CONTROLLER_OK;
}

// Update
GameControllerStatus game_update(Game* updatedGame) {
    sqlite3* db = db_open();
    LOG_INFO("UPDATE GAME_ID: %d", updatedGame->id_game);
    GameDaoStatus status = update_game_by_id(db, updatedGame);
    db_close(db);

    if (status == GAME_DAO_NOT_MODIFIED) {
        LOG_INFO("No changes detected for game %d, skipping update.", updatedGame->id_game);
        return GAME_CONTROLLER_OK;     // <-- NON è un errore
    }

    if (status != GAME_DAO_OK) {
        LOG_WARN("%s\n", return_game_dao_status_to_string(status));
        return GAME_CONTROLLER_DATABASE_ERROR;
    }

    return GAME_CONTROLLER_OK;
}

// Delete
GameControllerStatus game_delete(int64_t id_game) {
    sqlite3* db = db_open();
    GameDaoStatus status = delete_game_by_id(db, id_game);
    db_close(db);
    if (status != GAME_DAO_OK) {
        LOG_WARN("%s\n", return_game_dao_status_to_string(status));
        return status == GAME_DAO_NOT_FOUND ? GAME_CONTROLLER_NOT_FOUND : GAME_CONTROLLER_DATABASE_ERROR;
    }

    return GAME_CONTROLLER_OK;
}

// Read one with player info
GameControllerStatus game_find_one_with_player_info(int64_t id_game, GameWithPlayerNickname* retrievedGame) {
    sqlite3* db = db_open();
    GameDaoStatus status = get_game_by_id_with_player_info(db, id_game, retrievedGame);
    db_close(db);
    if (status != GAME_DAO_OK) {
        LOG_WARN("%s\n", return_game_dao_status_to_string(status));
        return status == GAME_DAO_NOT_FOUND ? GAME_CONTROLLER_NOT_FOUND : GAME_CONTROLLER_DATABASE_ERROR;
    }

    return GAME_CONTROLLER_OK;
}

// Read all with player info
GameControllerStatus game_find_all_with_player_info(GameWithPlayerNickname **retrievedGameArray, int* retrievedObjectCount) {
    sqlite3* db = db_open();
    GameDaoStatus status = get_all_games_with_player_info(db, retrievedGameArray, retrievedObjectCount);
    db_close(db);
    if (status != GAME_DAO_OK) {
        LOG_WARN("%s\n", return_game_dao_status_to_string(status));
        return GAME_CONTROLLER_DATABASE_ERROR;
    }

    return GAME_CONTROLLER_OK;
}