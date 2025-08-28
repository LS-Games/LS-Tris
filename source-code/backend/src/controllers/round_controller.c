#include <stdio.h>
#include <stdbool.h>

#include "round_controller.h"
#include "../entities/round_entity.h"

char find_horizontal_winner(char board[BOARD_MAX]) {
    char winner = NO_SYMBOL;

    // First row
    if (board[BOARD_ROWS*0 + 0] == board[BOARD_ROWS*0 + 1] && board[BOARD_ROWS*0 + 1] == board[BOARD_ROWS*0 + 2])
        winner = board[BOARD_ROWS*0 + 0];
    // Second row
    else if (board[BOARD_ROWS*1 + 0] == board[BOARD_ROWS*1 + 1] && board[BOARD_ROWS*1 + 1] == board[BOARD_ROWS*1 + 2])
        winner = board[BOARD_ROWS*1 + 0];
    // Third row
    else if (board[BOARD_ROWS*2 + 0] == board[BOARD_ROWS*2 + 1] && board[BOARD_ROWS*2 + 1] == board[BOARD_ROWS*2 + 2])
        winner = board[BOARD_ROWS*2 + 0];

    return winner;        
}

char find_vertical_winner(char board[BOARD_MAX]) {
    char winner = NO_SYMBOL;

    // First column
    if (board[BOARD_ROWS*0 + 0] == board[BOARD_ROWS*1 + 0] && board[BOARD_ROWS*1 + 0] == board[BOARD_ROWS*2 + 0])
        winner = board[BOARD_ROWS*0 + 0];
    // Second column
    else if (board[BOARD_ROWS*0 + 1] == board[BOARD_ROWS*1 + 1] && board[BOARD_ROWS*1 + 1] == board[BOARD_ROWS*2 + 1])
        winner = board[BOARD_ROWS*0 + 1];
    // Third column
    else if (board[BOARD_ROWS*0 + 2] == board[BOARD_ROWS*1 + 2] && board[BOARD_ROWS*1 + 2] == board[BOARD_ROWS*2 + 2])
        winner = board[BOARD_ROWS*0 + 2];

    return winner;        
}

char find_diagonal_winner(char board[BOARD_MAX]) {
    char winner = NO_SYMBOL;

    // Principal diagonal
    if (board[BOARD_ROWS*0 + 0] == board[BOARD_ROWS*1 + 1] && board[BOARD_ROWS*1 + 1] == board[BOARD_ROWS*2 + 2])
        winner = board[BOARD_ROWS*1 + 1];
    // Secondary diagonal
    else if (board[BOARD_ROWS*0 + 2] == board[BOARD_ROWS*1 + 1] && board[BOARD_ROWS*1 + 1] == board[BOARD_ROWS*2 + 0])
        winner = board[BOARD_ROWS*0 + 1];

    return winner;        
}

char find_winner(char board[BOARD_MAX]) {
    char winner = find_horizontal_winner(board);

    if (winner == NO_SYMBOL) {
        winner = find_vertical_winner(board);

        if (winner == NO_SYMBOL) {
            winner = find_diagonal_winner(board);
        }
    }

    return winner;
}

bool is_valid_move(char board[BOARD_MAX], int row, int col) {
    bool valid = false;

    char cell = get_round_board_cell(board, row, col);

    if (cell == NO_SYMBOL)
        valid = true;

    return valid;
}

bool make_move(char board[BOARD_MAX], int row, int col, char symbol) {
    bool success = false;

    if (is_valid_move(board, row, col)) {
        set_round_board_cell(board, row, col, symbol);
        success = true;
    } else {
        printf ("Not a valid move!\n");
    }

    return success;
}