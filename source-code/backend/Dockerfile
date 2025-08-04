# Dockerfile del backend

# Uso alpine come immagine per compilare l'app
FROM alpine:3.22.1 AS build-env

# Installo build-base meta package all'interno del container build-env
RUN apk add --no-cache build-base

# Vado nella directory /src
WORKDIR /src

# Copio tutti i files dalla directory corrente all'interno del container build-env
COPY . .

# Compilo il codice sorgente e genero il file eseguibile
RUN gcc main.c -o my-app


# Uso un altro container per eseguire l'app
FROM alpine:3.22.1

# Copio il file eseguibile dal container build-env al nuovo container
COPY --from=build-env /src/my-app /app/my-app

WORKDIR /app

# Esegui l'app
CMD ["/app/my-app"] 