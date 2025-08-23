# Dockerfile del backend

## Uso alpine come immagine per compilare l'app
FROM alpine:3.22.1 AS build-env

## Dipendenze del backend, installate all'interno del container build-env
    ### Installo build-base meta package
RUN apk add --no-cache build-base \
    
    ### Installa SQLite (non c'è bisogno della CLI `sqlite`)
    && apk add --no-cache sqlite-dev \

    ### Installa json-c
    && apk add --no-cache json-c-dev

## Vado nella directory /backend (se non esiste, la crea)
WORKDIR /backend

## Copio tutti i files dalla directory locale alla directory corrente all'interno del container build-env
COPY . .

## Eseguo il makefile in modalità release
RUN make release


## Uso un altro container per eseguire l'app
FROM alpine:3.22.1

## Dipendenze runtime del backend
RUN apk add --no-cache \
    sqlite-libs \
    json-c

## Vado nella directory /app (se non esiste, la crea)
WORKDIR /app

## Copio il file eseguibile dal container build-env al nuovo container
COPY --from=build-env /backend/bin/ls-tris my-app

## Eseguo l'app
CMD ["/app/my-app"] 