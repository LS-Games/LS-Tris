#include <stdbool.h>

#include "../entities/round_entity.h"

char find_horizontal_winner(char board[BOARD_MAX]);
char find_vertical_winner(char board[BOARD_MAX]);
char find_diagonal_winner(char board[BOARD_MAX]);
char find_winner(char board[BOARD_MAX]);
bool is_valid_move(char board[BOARD_MAX], int row, int col);
bool make_move(char board[BOARD_MAX], int row, int col, char symbol);