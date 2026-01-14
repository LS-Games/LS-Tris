# Production backend's Dockerfile

# ---- STAGE 1: Build C app ----
# Uso un'immagine Debian 12 Slim
# Doc: https://hub.docker.com/_/debian 
FROM debian:bookworm-slim AS build

# Dipendenze del backend, installate all'interno del container
    ## Evita pacchetti raccomandati e risponde si a tutte le domande
RUN apt-get update && apt-get install -y --no-install-recommends \

    ## Installa tool per la compilazione in C/C++
    build-essential \

    ## Installa gcc (se non incluso in build-essential)
    gcc \

    ## Installa json-c
    libjson-c-dev \

    ## Installa SQLite (non c'è bisogno della CLI sqlite3)
    libsqlite3-dev \

    ## Installa make (se non incluso in build-essential)
    make

# Spostiamoci nella directory `app` del container
WORKDIR /app

# Copio tutti i files dalla directory locale alla directory corrente all'interno del container
COPY . .

## Build del progetto in modalità release
RUN make release


# ---- STAGE 2: Runtime ----
# Uso un altro container per eseguire l'app
FROM debian:bookworm-slim

# Dipendenze runtime del backend
    ## Evita pacchetti raccomandati e risponde si a tutte le domande
RUN apt-get update && apt-get install -y --no-install-recommends \

    ## Installa SQLite (e la CLI sqlite3)
    sqlite3 libsqlite3-0 \

    ## Installa json-c
    libjson-c5 \

    ## Clean della cache
    && apt-get clean
    
# Spostiamoci nella directory `/app` del container
WORKDIR /app

# Copio il file eseguibile dal container build al nuovo container
COPY --from=build /app/bin/ls-tris /app/bin/ls-tris

# Copio lo script `entrypoint.sh` e lo rendo un file eseguibile
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

# Copio gli script SQL
COPY db/scheme.sql db/populate_db.sql /app/db/

# Dichiara di voler esporre la porta del server C
# Avverte solo Docker che all'interno c'è un processo in ascolto sulla porta
EXPOSE 5050

## Eseguo l'app
ENTRYPOINT ["/entrypoint.sh"]
CMD ["./bin/ls-tris"]