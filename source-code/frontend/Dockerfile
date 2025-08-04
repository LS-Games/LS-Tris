# Dockerfile del frontend

# Uso un'immagine base che contiene Node.js, nvm e poche altre cose
# Doc: https://hub.docker.com/_/node
FROM node:22.16.0-slim


# Installiamo tutto ciò che è necessario per il frontend

## Dipendenze del frontend
    ### Installa Typescript
RUN npm install -g typescript \

    ### Installa SASS
    && npm install -g sass \

    ### Installa Angular
    && npm install -g @angular/cli