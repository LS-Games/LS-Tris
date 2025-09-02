#include <stdio.h>
#include <inttypes.h>

#include "player_entity.h"

void print_player(const Player *p) {
    if (!p) {
        printf("Player: (NULL)\n");
        return;
    }

    printf("Player {\n");
    printf("  id_player: %" PRId64 "\n", p->id_player);
    printf("  nickname: \"%s\"\n", p->nickname);
    printf("  email: \"%s\"\n", p->email);
    printf("  password: \"%s\"\n", p->password);
    printf("  current_streak: %d\n", p->current_streak);
    printf("  max_streak: %d\n", p->max_streak);
    printf("  registration_date: \"%s\"\n", p->registration_date);
    printf("}\n");
}

void print_player_inline(const Player *p) {
    if (!p) {
        printf("Player(NULL)\n");
        return;
    }
    printf("Player[id=%" PRId64 ", nick=%s, email=%s, streak=%d/%d, reg=%s]\n",
            p->id_player,
            p->nickname,
            p->email,
            p->current_streak,
            p->max_streak,
            p->registration_date);
}