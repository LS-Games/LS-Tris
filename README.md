# LS-Tris

Repository dedicata al progetto di Laboratorio di Sistemi Operativi 2024/2025.

ATTENZIONE! Sono stati predisposti degli script (sia [Script Bash](./scripts/linux/) che [Script CMD](./scripts/windows/)) per eseguire e rimuovere velocemente i containers definiti tramite Docker Compose.

## Indice

* [Creazione dei Docker Containers](#creazione-dei-docker-containers)
  + [Development Containers](#development-containers)
  + [Production Containers](#production-containers)
* [Rimozione dei Docker Container](#rimozione-dei-docker-container)

## Creazione dei Docker Containers

Nella configurazione di questa repository, sono stati creati tre diversi Docker Compose files. Lo scopo di questa scelta è quello di avere una separazione netta tra le macrocomponenti del software, in modo da favorire la manutenibilità e riusabilità. In particolare, distinguiamo i seguenti Docker Compose files:

- [docker-compose.backend.yml](.devcontainer/docker-compose.backend.yml), che contiene tutti i servizi relativi al backend;
- [docker-compose.frontend.yml](.devcontainer/docker-compose.frontend.yml), che contiene tutti i servizi relativi al frontend;
- [docker-compose.dev.yml](.devcontainer/docker-compose.dev.yml), che contiene tutti i servizi necessari solo nella fase di sviluppo del software (sia frontend che backend).

**ATTENZIONE! I seguenti comandi sono pensati per un ambiente Host Linux, tuttavia è possibile facilmente tradurli in ambiente Host Windows sostituendo il carattere di fine linea `\` con `^`.**

E' possibile avviare il Docker Compose nel seguente modo:

1. Spostarsi nella directory [.devcontainer](./.devcontainer)
1. Eseguire il comando:

    ```Bash
    docker compose --profile <NomeProfilo> \
        -f docker-compose.backend.yml \
        -f docker-compose.frontend.yml \
        -f docker-compose.dev.yml \
        --project-name <NomeCompose> \
        up -d
    ```

NOTA! E' importante impostare i profili che si desidera attivare, poichè di default saranno avviati solo i container non associati ad alcun profilo.

E' stato predisposto un file [.sample-env](.devcontainer/.sample-env), da duplicare e rinominare in `.env`, che contiene le variabili d'ambiente necessarie al corretto funzionamento dei Docker Containers.

ATTENZIONE! Riferirsi anche ai README di [backend](./source-code/backend/), [bridge](./source-code/bridge/) e [frontend](./source-code/frontend/) per informazioni supplementari (ad esempio, come generare un database iniziale per il backend).

### Development Containers

Per utilizzare i Development Containers è necessario spostarsi nella directory [.devcontainer](./.devcontainer) e avviare i Docker Compose files con il profilo `dev`:

```Bash
docker compose --profile dev \
    -f docker-compose.backend.yml \
    -f docker-compose.frontend.yml \
    -f docker-compose.dev.yml \
    --project-name ls-tris-dev \
    up -d
```

ATTENZIONE! Se si sta avviando i Development Containers tramite [devcontainer.json](.devcontainer/devcontainer.json), è necessario impostare il profilo tramite la variabile d'ambiente `COMPOSE_PROFILES=dev`, già settata nel file [.sample-env](.devcontainer/.sample-env).

### Production Containers

Per utilizzare i Production Containers è necessario spostarsi nella directory [.devcontainer](./.devcontainer) e avviare i Docker Compose files con il profilo `prod`:

```Bash
docker compose --profile prod \
    -f docker-compose.backend.yml \
    -f docker-compose.frontend.yml \
    -f docker-compose.dev.yml \
    --project-name ls-tris-prod \
    up -d
```

ATTENZIONE! Ricordarsi che, di default, il profilo selezionato in [.sample-env](.devcontainer/.sample-env) è il profilo `dev`. Sarà necessario modificare i profili impostati nel file `.env` locale per evitare comportamenti inaspettati.

## Rimozione dei Docker Container

Per rimuovere i Docker Containers avviati tramite Docker Compose files, è necessario spostarsi nella directory [.devcontainer](./.devcontainer) ed eseguire il comando:

```Bash
docker compose --profile <NomeProfilo> \
    -f docker-compose.backend.yml \
    -f docker-compose.frontend.yml \
    -f docker-compose.dev.yml \
    --project-name <NomeCompose> \
    down
```

Per brevità, non vengono riportati i singoli comandi per le due modalità di esecuzione. Tuttavia, sono stati utilizzati negli scripts definiti nell'apposita [directory](./scripts/).
