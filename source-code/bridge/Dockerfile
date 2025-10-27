# Usa un'immagine leggera di Node
FROM node:20-alpine

# Crea la directory di lavoro
WORKDIR /bridge

# Copia i file di definizione pacchetti
# Con --omit=dev non installiamo le dipendenze per lo sviluppo 
COPY package*.json ./
RUN npm install --omit=dev 

# Copia il resto del progetto
COPY . .

# Espone la porta WS (la porta non esce fuori dal container)
# Avverte solo Docker che all'interno c'è un processo in ascolto sulla porta

EXPOSE 8080

# Comando di avvio
CMD ["node", "bridge.js"]
