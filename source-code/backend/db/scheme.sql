PRAGMA foreign_keys = ON;

-- =========================================================
-- PLAYER
-- =========================================================
CREATE TABLE Player (
    id_player           INTEGER PRIMARY KEY AUTOINCREMENT,
    nickname            TEXT    NOT NULL UNIQUE,
    email               TEXT    NOT NULL UNIQUE,
    password            TEXT    NOT NULL,
    current_streak      INTEGER NOT NULL DEFAULT 0,
    max_streak          INTEGER NOT NULL DEFAULT 0,
    registration_date   TEXT    NOT NULL
);

-- =========================================================
-- GAME
-- =========================================================
CREATE TABLE Game (
    id_game     INTEGER PRIMARY KEY AUTOINCREMENT,
    id_creator  INTEGER NOT NULL,
    id_owner    INTEGER NOT NULL,
    state       TEXT NOT NULL CHECK (state IN ('new', 'active', 'waiting', 'finished')),
    created_at  TEXT NOT NULL,

    FOREIGN KEY (id_creator) REFERENCES Player(id_player)
        ON DELETE RESTRICT ON UPDATE CASCADE,
    FOREIGN KEY (id_owner) REFERENCES Player(id_player)
        ON DELETE RESTRICT ON UPDATE CASCADE
);

-- =========================================================
-- ROUND
-- =========================================================
CREATE TABLE Round (
    id_round    INTEGER PRIMARY KEY AUTOINCREMENT,
    id_game     INTEGER NOT NULL,
    state       TEXT    NOT NULL CHECK (state IN ('active', 'finished')),
    start_time  INTEGER NOT NULL,     -- unix timestamp (seconds)
    end_time    INTEGER DEFAULT NULL, -- NULL while round is active

    board       TEXT NOT NULL,

    FOREIGN KEY (id_game) REFERENCES Game(id_game)
        ON DELETE CASCADE ON UPDATE CASCADE
);

-- =========================================================
-- PLAY
-- =========================================================
CREATE TABLE Play (
    id_player       INTEGER NOT NULL,
    id_round        INTEGER NOT NULL,
    result          TEXT CHECK (result IN ('win', 'lose', 'draw') OR result IS NULL),
    player_number   INTEGER NOT NULL CHECK (player_number IN (1, 2)),

    PRIMARY KEY (id_player, id_round),
    FOREIGN KEY (id_round) REFERENCES Round(id_round)
        ON DELETE CASCADE ON UPDATE CASCADE,
    FOREIGN KEY (id_player) REFERENCES Player(id_player)
        ON DELETE CASCADE ON UPDATE CASCADE
);

-- =========================================================
-- PARTICIPATION REQUEST
-- =========================================================
CREATE TABLE Participation_request (
    id_request  INTEGER PRIMARY KEY AUTOINCREMENT,
    id_player   INTEGER NOT NULL,
    id_game     INTEGER NOT NULL,
    created_at  TEXT NOT NULL,
    state       TEXT NOT NULL CHECK (state IN ('pending', 'accepted', 'rejected')),

    FOREIGN KEY (id_player) REFERENCES Player(id_player)
        ON DELETE CASCADE ON UPDATE CASCADE,
    FOREIGN KEY (id_game) REFERENCES Game(id_game)
        ON DELETE CASCADE ON UPDATE CASCADE
);
