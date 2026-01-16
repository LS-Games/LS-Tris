# Backend

Backend dedicato al progetto LS-Tris.

Il compilatore utilizzato è `gcc 14.2.0`.

Ogni comando si intende eseguito dalla root directory del progetto, [backend](.).

## Indice

* [Third-party Dependencies](#third-party-dependencies)
* [Scripts](#scripts)
* [SQLite](#sqlite)
    + [Creazione dello schema da terminale](#creazione-dello-schema-da-terminale)
    + [Visualizzazione del database da terminale](#visualizzazione-del-database-da-terminale)
    + [Popolazione del database da terminale](#popolazione-del-database-da-terminale)
* [Struttura del progetto](#struttura-del-progetto)

## Third-party Dependencies

Il backend è stato costruito con le seguenti dipendenze:

* [SQLite](https://sqlite.org/);
* [json-c](https://github.com/json-c/json-c/wiki).

## Scripts

Per compilare ed eseguire il backend, è stato predisposto un [Makefile](./Makefile). Gli script definiti sono i seguenti:

* `make`, `make all` o `make debug` compila il progetto in modalità debug;
* `make install` esegue `all` e installa l'eseguibile in `/usr/local/bin/`, impostando i permessi a `755`;
* `make clean` elimina le directory `./bin/` e `./build`;
* `make release` esegue `clean` e compila il progetto in modalità ottimizzata per la produzione;
* `make run` esegue l'eseguibile in `./bin/`.

## SQLite

### Creazione dello schema da terminale

Possiamo creare un file `scheme.sql`, contenente lo schema del database in linguaggio SQL. Spostiamoci nella working directory [backend](.) e creiamo le tabelle con il comando:

```bash
mkdir -p ./db/data
sqlite3 ./db/data/database.sqlite < ./db/scheme.sql
```

### Visualizzazione del database da terminale

Possiamo visualizzare il database da terminale. Per farlo, spostiamoci nella working directory [backend](.) ed eseguiamo i seguenti comandi:

```bash
sqlite3
.read ./db/scheme.sql
.tables # Ci appaiono tutte le tabelle
.schema Player # Ci da' nel dettaglio la tabella "Player"
```

Tuttavia, tramite l'estensione [SQLite](https://marketplace.visualstudio.com/items?itemName=alexcvzz.vscode-sqlite) per VSCode, è possibile visualizzare il database in modo più intuitivo dalla scheda *SQLITE EXPLORER* in basso a sinistra.

### Popolazione del database da terminale

Possiamo creare un file `populate_db.sql`, contenente le istruzioni SQL per popolare il database. Spostiamoci nella working directory [backend](.) e popoliamo il database con il comando:

```bash
sqlite3 ./db/data/database.sqlite < ./db/populate_db.sql
```

## Struttura del progetto

Ultimo aggiornamento: 16/01/2026

```text
/backend
│
├── bin/                                    @ Directory contenete gli eseguibili finali
│   └── ls-tris                                 # App eseguibile
│
├── build/                                  @ Directory contenete gli artifacts di compilazione
│   └──  ...
|
├── db/                                     @ Directory contenente i files per il database
│   ├── populate_db.sql                         # Popolazione del database
│   ├── schema.sql                              # Schema SQL
│   └── data/                                   @ Directory contenente i database utilizzati dall'app
│       └── database.sql                            # Database SQLite
│
├── include/                                @ Directory contenete gli header pubblici
│   └── debug_log.h                             # Definizione delle direttive di logging
│
├── src/                                    @ Directory contenente il codice sorgente
│   │
│   ├── controllers/                            @ Directory contenente la logica di business dell'app (funzionalità e operazioni CRUD)
│   │   └──  ...                                    # Logica turni, validazione mosse, check vittoria...
│   │
│   ├── dao/                                    @ Directory contenente la logica di comunicazione tra app e database
│   │   ├── dto/                                    @ Definizione di strutture custom di comunicazione con layer di persistenza
│   │   │   └── ...
│   │   └── sqlite/                                 @ Definizione del DAO per SQLite
│   │       ├── db_connection_sqlite.c / .h             # Connessione al database
│   │       └── ...                                     # Operazioni CRUD per le entità del dominio
│   │
│   ├── dto/                                    @ Directory contenente la definizione di strutture custom di comunicazione con layer di rete
│   │   └──  ...
│   │
│   ├── entities/                               @ Directory contenente la definizione di strutture per le entità del dominio
│   │   └──  ...                                    # Game, Participation Request, Play, Player, Round (Tipi e logica di stampa)
│   │
│   ├── json-parser/                            @ Directory contenente la logica di parsing da DTO a JSON
│   │   └──  ...
│   │
│   ├── server/                                 @ Directory contenente la logica di orchestrazione dei clients, HTTP Requests e app sessions
│   │   ├── router.c / .h                           # Definizione del router in base alla HTTP Request, costruzione e invio della HTTP Response
│   │   ├── server.c / .h                           # Clients management tramite Threads e Sockets
│   │   └── session_manager.c / .h                  # Gestione della sessione dei giocatori
│   │
│   └── main.c                                  # Bootstrap 
│
├── .dockerignore                           # File di definizione della ignore-list del Dockerfile
├── .gitignore                              # File di definizione della ignore-list del sistema di versioning
├── Dockerfile                              # Definizione del Docker container di produzione
├── entrypoint.sh                           # Entrypoint del Docker container di produzione
├── Makefile                                # Script di build
└── README.md                               # Questo README
```
