#include <stdbool.h>

#include "../entities/play_entity.h"

typedef enum {
    PLAY_CONTROLLER_OK = 0,
    //PLAY_CONTROLLER_INVALID_INPUT,
    PLAY_CONTROLLER_NOT_FOUND,
    //PLAY_CONTROLLER_STATE_VIOLATION,
    PLAY_CONTROLLER_DATABASE_ERROR,
    //PLAY_CONTROLLER_CONFLICT,
    //PLAY_CONTROLLER_FORBIDDEN,
    //PLAY_CONTROLLER_INTERNAL_ERROR
} PlayControllerStatus;


// ===================== CRUD Operations =====================

PlayControllerStatus play_create(Play* playToCreate);
PlayControllerStatus play_find_all(Play** retrievedPlayArray, int* retrievedObjectCount);
PlayControllerStatus play_find_one(int id_play, int id_round, Play* retrievedPlay);
PlayControllerStatus play_update(Play* updatedPlay);
PlayControllerStatus play_delete(int id_play, int id_round);

PlayControllerStatus play_find_all_by_round(Play** retrievedPlayArray, int64_t id_round, int* retrievedObjectCount);