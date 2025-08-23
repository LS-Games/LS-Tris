# Dockerfile del workspace

# Uso un'immagine base che contiene Node.js, git, nvm e altro
# Doc: https://hub.docker.com/r/microsoft/devcontainers-javascript-node
FROM mcr.microsoft.com/devcontainers/javascript-node:22


# Installiamo tutto ciò che è necessario sia per il frontend che per il backend

## Dipendenze del backend
    ### Update dei package installati     
RUN apt update \

    ### Installa il C debugger
    && apt-get install gdb -y \

    ### Installa SQLite (e la CLI sqlite3)
    && apt install sqlite3 libsqlite3-dev -y \

    ### Installa json-c
    && apt install libjson-c-dev -y \

    ### Clean della cache
    && apt clean

## Dipendenze del frontend
    ### Installa Typescript
RUN npm install -g typescript \

    ### Installa SASS
    && npm install -g sass \

    ### Installa Angular
    && npm install -g @angular/cli