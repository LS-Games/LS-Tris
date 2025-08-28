#include <stdbool.h>

#include "../entities/round_entity.h"

char find_winner(char board[BOARD_MAX]);
bool is_valid_move(char board[BOARD_MAX], int row, int col);
bool make_move(char board[BOARD_MAX], int row, int col, char symbol);

bool start_round(int id_game, int64_t duration);