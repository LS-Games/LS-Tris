--Foreign key contraints are disabled by default 

PRAGMA foreign_keys = ON;

CREATE TABLE Player (
    id_player           INTEGER PRIMARY KEY AUTOINCREMENT, 
    nickname            TEXT    NOT NULL UNIQUE,
    email               TEXT    NOT NULL UNIQUE, 
    password            TEXT    NOT NULL,
    current_streak      INTEGER NOT NULL DEFAULT 0,
    max_streak          INTEGER NOT NULL DEFAULT 0, 
    registration_date   TEXT NOT NULL
);

CREATE TABLE Game (
    id_game             INTEGER PRIMARY KEY AUTOINCREMENT, 
    id_creator          INTEGER NOT NULL,
    id_owner            INTEGER NOT NULL,
    state               TEXT    NOT NULL CHECK (state IN ('new', 'active', 'waiting', 'finished')), 
    created_at          TEXT    NOT NULL, 
    FOREIGN KEY (id_creator) REFERENCES Player(id_player) ON DELETE RESTRICT,
    FOREIGN KEY (id_owner) REFERENCES Player(id_player) ON DELETE RESTRICT
);

CREATE TABLE Round (
    id_round            INTEGER PRIMARY KEY AUTOINCREMENT,
    id_game             INTEGER NOT NULL, 
    state               TEXT    NOT NULL CHECK (state IN ('pending', 'active', 'finished')), 
    duration            INTEGER NOT NULL, 
    FOREIGN KEY (id_game) REFERENCES Game(id_game) ON DELETE CASCADE
);

-- We should add an attribute "role" to limit game to two player
-- So we can use UNIQUE()

CREATE TABLE Play (
    id_player           INTEGER NOT NULL, 
    id_round            INTEGER NOT NULL, 
    result              TEXT NOT NULL CHECK (result IN ('win', 'lose', 'draw')), 
    -- role             TEXT NOT NULL,         -- We mean X or O
    -- UNIQUE (id_round, role),
    PRIMARY KEY (id_player, id_round),
    FOREIGN KEY (id_round) REFERENCES Round(id_round) ON DELETE CASCADE,
    FOREIGN KEY (id_player) REFERENCES Player(id_player) ON DELETE CASCADE 
);

CREATE TABLE Participation_request (
    id_request          INTEGER PRIMARY KEY AUTOINCREMENT,
    id_player           INTEGER NOT NULL,
    id_game             INTEGER NOT NULL, 
    created_at          TEXT NOT NULL,
    state               TEXT NOT NULL CHECK (state IN ('pending', 'accepted', 'rejected', 'canceled')), 
    FOREIGN KEY (id_player) REFERENCES Player(id_player) ON DELETE CASCADE,
    FOREIGN KEY (id_game) REFERENCES Game(id_game) ON DELETE CASCADE  
);
