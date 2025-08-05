# LS-Tris

Repository dedicata al progetto di Laboratorio di Sistemi Operativi 2024/2025.

## Indice

* [Creazione dei Docker Containers](#creazione-dei-docker-containers)
    + [Development Containers](#development-containers)
    + [Production Containers](#production-containers)

## Creazione dei Docker Containers

Nella configurazione di questa repository, sono stati creati tre diversi Docker Compose files. Lo scopo di questa scelta è quello di avere una separazione netta tra le macrocomponenti del software, in modo da favorire la manutenibilità e riusabilità. In particolare, distinguiamo i seguenti Docker Compose files:

- [docker-compose.backend.yml](.devcontainer/docker-compose.backend.yml), che contiene tutti i servizi relativi al backend;
- [docker-compose.frontend.yml](.devcontainer/docker-compose.frontend.yml), che contiene tutti i servizi relativi al frontend;
- [docker-compose.dev.yml](.devcontainer/docker-compose.dev.yml), che contiene tutti i servizi necessari solo nella fase di sviluppo del software (sia frontend che backend).

E' possibile avviare il Docker Compose nel seguente modo:

1. Spostarsi nella directory `.devcontainer`
1. Eseguire il comando:

    ```Bash
    docker compose --profile <NomeProfilo> -f docker-compose.backend.yml -f docker-compose.frontend.yml -f docker-compose.dev.yml up -d
    ```

NOTA! E' importante impostare i profili che si desidera attivare, poichè di default saranno avviati solo i container non associati ad alcun profilo.

E' stato predisposto un file [.sample-env](.devcontainer/.sample-env), da duplicare e rinominare in `.env`, che contiene le variabili d'ambiente necessarie al corretto funzionamento dei Docker Containers.

### Development Containers

Per utilizzare i Development Containers è necessario avviare i Docker Compose files con il profilo `dev`:

```Bash
docker compose --profile dev -f docker-compose.backend.yml -f docker-compose.frontend.yml -f docker-compose.dev.yml up -d
```

ATTENZIONE! Se si sta avviando i Development Containers tramite [devcontainer.json](.devcontainer/devcontainer.json), è necessario impostare il profilo tramite la variabile d'ambiente `COMPOSE_PROFILES=dev`, già settata nel file [.sample-env](.devcontainer/.sample-env).

### Production Containers

Per utilizzare i Production Containers è necessario avviare i Docker Compose files con il profilo `prod`:

```Bash
docker compose --profile prod -f docker-compose.backend.yml -f docker-compose.frontend.yml -f docker-compose.dev.yml up -d
```

ATTENZIONE! Ricordarsi che, di default, il profilo selezionato in [.sample-env](.devcontainer/.sample-env) è il profilo `dev`. Sarà necessario modificare i profili impostati nel file `.env` locale per evitare comportamenti inaspettati.
