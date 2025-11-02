# Uso un'immagine base che contiene Node.js, nvm e poche altre cose
# Doc: https://hub.docker.com/_/node
FROM node:22.16.0-slim AS build

# Spostiamoci nella directory `app` del container
WORKDIR /app

# Dipendenze del bridge, installate all'interno del container
    ## Copia sia package-lock.json che package.json nella WORKDIR del container (da' errore se mancano o sono incoerenti)
COPY package*.json ./
    ## Clean install delle dipendenze (ottimizzato per build riproducibili)
    ## Con --omit=dev non installiamo le dipendenze per lo sviluppo 
RUN npm ci --omit=dev 

# Copio tutti i files dalla directory locale alla directory corrente all'interno del container
COPY . .

# Dichiara di voler esporre la porta HTTP e WebSocket
# Avverte solo Docker che all'interno c'è un processo in ascolto sulle porte
EXPOSE 3001 3002

# Avvia il bridge
CMD ["npm", "start"]
