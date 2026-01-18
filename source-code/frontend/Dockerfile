# Production frontend's Dockerfile

# ---- STAGE 1: Build Angular app ----
# Uso un'immagine base che contiene Node.js, nvm e poche altre cose
# Doc: https://hub.docker.com/_/node
FROM node:22.16.0-slim AS build

# Spostiamoci nella directory `app` del container
WORKDIR /app

# Dipendenze del frontend, installate all'interno del container
    ## Copia sia package-lock.json che package.json nella WORKDIR del container (da' errore se mancano o sono incoerenti)
COPY package*.json ./
    ## Clean install delle dipendenze (ottimizzato per build riproducibili)
RUN npm ci

# Copio tutti i files dalla directory locale alla directory corrente all'interno del container
COPY . .
# Build del progetto in modalità di produzione
RUN npm run build


# ---- STAGE 2: Serve con NGINX ----
# Uso un'immagine basata su alpine per il reverse proxy Nginx
# Doc: https://hub.docker.com/_/nginx
FROM nginx:1.27-alpine AS production

# Copia la build Angular dal container build al path servito da NGINX
COPY --from=build /app/dist/frontend/browser /usr/share/nginx/html

# Copio il file template di env variables
COPY --from=build /app/src/assets/env.template.js /usr/share/nginx/html/assets/env.template.js

# Copio lo script `entrypoint.sh` e lo rendo un file eseguibile
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

# Copia la configurazione NGINX custom
COPY nginx.conf /etc/nginx/conf.d/default.conf

# Dichiara di voler esporre la porta NGINX
# Avverte solo Docker che all'interno c'è un processo in ascolto sulla porta
EXPOSE 80

# Avvia la web app
ENTRYPOINT ["/entrypoint.sh"]
CMD ["nginx", "-g", "daemon off;"]