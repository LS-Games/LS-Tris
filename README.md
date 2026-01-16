# LS-Tris

Repository dedicata al progetto di Laboratorio di Sistemi Operativi 2024/2025.

ATTENZIONE! Sono stati predisposti degli script (sia [Script Bash](./scripts/linux/) che [Script CMD](./scripts/windows/)) per eseguire e rimuovere velocemente i containers definiti tramite Docker Compose.

La documentazione è disponibile nella directory [documentation](./documentation/). Clicca [qui](./documentation/Documentazione%20LS-Tris.pdf) per un link rapido.

## Indice

* [IDE consigliati](#ide-consigliati)
* [System requirements](#system-requirements)
    + [Installazione tramite Docker Containers](#installazione-tramite-docker-containers)
        - [Development Containers](#development-containers)
        - [Production Containers](#production-containers)
    + [Rimozione dei Docker Containers](#rimozione-dei-docker-containers)

## IDE consigliati

Per lo sviluppo del progetto si consiglia [Visual Studio Code](https://code.visualstudio.com/). In particolare, affiancato allo sviluppo del progetto tramite [DevContainer](https://code.visualstudio.com/docs/devcontainers/containers)

Maggiori informazioni ai seguenti articoli:

* [Development Containers](https://containers.dev/)
* [Developing inside a Container](https://code.visualstudio.com/docs/devcontainers/containers)

## System requirements

ATTENZIONE! In questa sezione verrano definiti solo i prerequisiti di sistema, cioè quei software che devono essere installati sul sistema host per far funzionare l'intera app.

Riferirsi ai *README.md* dei singoli progetti per le package dependencies, scripts predefiniti e altre informazioni utili per eseguire il progetto. Un link diretto ai *README.md* è disponibile qui:

* [Frontend](./source-code/frontend/README.md);
* [Bridge](./source-code/bridge/README.md);
* [Backend](./source-code/backend/README.md).

### Installazione tramite Docker Containers

Nella configurazione di questa repository, sono stati creati tre diversi Docker Compose files. Lo scopo di questa scelta è quello di avere una separazione netta tra le macrocomponenti del software, in modo da favorire la manutenibilità e riusabilità. In particolare, distinguiamo i seguenti Docker Compose files:

* [docker-compose.backend.yml](.devcontainer/docker-compose.backend.yml), che contiene tutti i servizi relativi al backend;
* [docker-compose.frontend.yml](.devcontainer/docker-compose.frontend.yml), che contiene tutti i servizi relativi al frontend;
* [docker-compose.dev.yml](.devcontainer/docker-compose.dev.yml), che contiene tutti i servizi necessari solo nella fase di sviluppo del software (sia frontend che backend).

**ATTENZIONE! I seguenti comandi sono pensati per un ambiente Host Linux, tuttavia è possibile facilmente tradurli in ambiente Host Windows sostituendo il carattere di fine linea `\` con `^`.
NOTA! Usare `^` per cmd, `` ` `` per PowerShell.**

E' possibile avviare il Docker Compose nel seguente modo:

1. Spostarsi nella directory [.devcontainer](./.devcontainer)
1. Eseguire il comando:

    ```Bash
    docker compose --profile <NOME_PROFILO> \
        -f docker-compose.backend.yml \
        -f docker-compose.frontend.yml \
        -f docker-compose.dev.yml \
        --project-name <NOME_COMPOSE> \
        up -d
    ```

NOTA! E' importante impostare i profili che si desidera attivare, poichè di default saranno avviati solo i container non associati ad alcun profilo.

E' stato predisposto un file [.sample-env](.devcontainer/.sample-env), da duplicare e rinominare in `.env`, che contiene le variabili d'ambiente necessarie al corretto funzionamento dei Docker Containers.

#### Development Containers

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

#### Production Containers

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

### Rimozione dei Docker Containers

Per rimuovere i Docker Containers avviati tramite Docker Compose files, è necessario spostarsi nella directory [.devcontainer](./.devcontainer) ed eseguire il comando:

```Bash
docker compose --profile <NOME_PROFILO> \
    -f docker-compose.backend.yml \
    -f docker-compose.frontend.yml \
    -f docker-compose.dev.yml \
    --project-name <NOME_COMPOSE> \
    down
```

Per brevità, non vengono riportati i singoli comandi per le due modalità di esecuzione. Tuttavia, sono stati utilizzati negli scripts definiti nell'apposita [directory](./scripts/).
