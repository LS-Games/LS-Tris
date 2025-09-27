#include <stdio.h>
#include <inttypes.h>

#include "player_entity.h"

void print_player(const Player *p) {
    if (!p) {
        printf("Player: (NULL)\n");
        return;
    }

    // Compute time in UCT
    struct tm tm_utc;
    char buffer[26];
    gmtime_r(&p->registration_date, &tm_utc);
    asctime_r(&tm_utc, buffer);

    printf("Player {\n");
    printf("  id_player: %" PRId64 "\n", p->id_player);
    printf("  nickname: \"%s\"\n", p->nickname);
    printf("  email: \"%s\"\n", p->email);
    printf("  password: \"%s\"\n", p->password);
    printf("  current_streak: %d\n", p->current_streak);
    printf("  max_streak: %d\n", p->max_streak);
    printf("  registration_date: \"%s\"\n", buffer);
    printf("}\n");
}

void print_player_inline(const Player *p) {
    if (!p) {
        printf("Player(NULL)\n");
        return;
    }

    // Compute time in UCT
    struct tm tm_utc;
    char buffer[26];
    gmtime_r(&p->registration_date, &tm_utc);
    asctime_r(&tm_utc, buffer);

    printf("Player[id=%" PRId64 ", nick=%s, email=%s, pass=%s, streak=%d/%d, reg=%s]\n",
            p->id_player,
            p->nickname,
            p->email,
            p->password,
            p->current_streak,
            p->max_streak,
            buffer);
}