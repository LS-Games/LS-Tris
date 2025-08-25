# Backend

## SQLite

### Creazione dello schema da terminale

Nella directory [backend/src/db/](./src/db/), creiamo un file `scheme.sql`, contenente lo schema del database in linguaggio SQL. Spostiamoci nella working directory [backend](.). Creeremo le tabelle con il comando:

```bash
sqlite3 ./data/database.sqlite < ./src/db/scheme.sql
```

### Visualizzazione del database da terminale

Possiamo visualizzare il database da terminale. Per farlo, spostiamoci nella directory [backend](.) ed eseguiamo i seguenti comandi:

```bash
sqlite3
.read db/scheme.sql
.tables # Ci appaiono tutte le tabelle
.schema Player # Ci da' nel dettaglio la tabella "Player"
```

Tuttavia, tramite l'estensione [SQLite](https://marketplace.visualstudio.com/items?itemName=alexcvzz.vscode-sqlite) per VSCode è possibile visualizzare il database in modo più intuitivo dalla scheda *SQLITE EXPLORER* in basso a sinistra.

### Popolazione del database da terminale

Nella directory [backend/src/db/](./src/db/), creiamo un file `populate_db.sql`, contenente le istruzioni SQL per popolare il database. Popoleremo il database con il comando:

```bash
sqlite3 backend/data/database.sqlite < backend/src/db/populate_db.sql
```
