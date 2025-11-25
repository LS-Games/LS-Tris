# Frontend

Questo progetto è stato generato utilizzando [Angular CLI](https://github.com/angular/angular-cli) versione `20.0.4`.

L'interprete utilizzato è `node 22.19.0`.

Ogni comando si intende eseguito dalla root directory del progetto, [frontend](.).

## Indice

* [Package Dependencies](#package-dependencies)
* [Scripts](#scripts)
* [Development server](#development-server)
* [Code scaffolding](#code-scaffolding)
* [Building](#building)
* [Eseguire unit tests](#eseguire-unit-tests)
* [Eseguire end-to-end tests](#eseguire-end-to-end-tests)
* [Risorse aggiuntive](#risorse-aggiuntive)

## Package Dependencies

Il frontend è stato costruito con le dipendenze definite nel `package.json`.

Una volta impostata la versione di node corretta (lo verifichiamo con `node --version`), installiamo le package dependencies con il comando:

```bash
npm install
```

## Scripts

Per eseguire il frontend, è stato predisposto un [package.json](./package.json). Gli script definiti sono i seguenti:

* `npm run generate-env` esegue uno script per generare dinamicamente un file di configurazione delle variabili d'ambiente;
* `npm start` o `npm run start` esegue `generate-env` ed esegue il progetto in modalità sviluppo (variante `npm run start:docker` per sviluppo in ambiente Docker);
* `npm run build` esegue `generate-env` e compila il progetto in modalità sviluppo;
* `npm run watch` esegue `generate-env`, compila il progetto in modalità sviluppo e resta in ascolto per ricompilare automaticamente al variare dei file;
* `npm run build:prod` esegue `generate-env` e compila il progetto in modalità produzione;
* `npm run serve` serve la build del progetto tramite `http-server` e disabilitando la cache;
* `npm run test` esegue test unitari.

## Development server

Per avviare un server locale di sviluppo, esegui:

```bash
ng serve
```

In alternativa, è possibile usare `npm start` o `npm run start`.

Una volta avviato, apri il browser su `http://localhost:4200/`. L'applicazione si ricaricherà automaticamente ogni volta che modifichi un file sorgente.

## Code scaffolding

Angular CLI include potenti strumenti per la generazione automatica del codice. Per creare un nuovo componente, esegui:

```bash
ng generate component component-name
```

Per un elenco completo degli schemi disponibili (come `components`, `directives` o `pipes`), esegui:

```bash
ng generate --help
```

## Building

Per compilare il progetto, esegui:

```bash
ng build
```

In alternativa, è possibile usare `npm run build`.

Questo compilerà il tuo progetto e salverà i file compilati verranno nella directory `dist/`. Di default, la build di produzione è ottimizzata per prestazioni e velocità.

## Eseguire unit tests

Per eseguire i test unitari con il test runner [Karma](https://karma-runner.github.io), usa il comando seguente:

```bash
ng test
```

In alternativa, è possibile usare `npm run test`.

## Eseguire end-to-end tests

Per eseguire test end-to-end (e2e), esegui:

```bash
ng e2e
```

Angular CLI non include un end-to-end testing framework di default. Puoi scegliere quello che preferisci.

## Risorse aggiuntive

Per maggiori informazioni sull'uso di Angular CLI, con una documentazione dettagliata dei comandi, visita la pagina [Angular CLI Overview and Command Reference](https://angular.dev/tools/cli).
