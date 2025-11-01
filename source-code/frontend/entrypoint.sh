#!/bin/sh

# -e: Se un comando fallisce, la shell esce
# -u: Se usiamo una variabile non definita, la shell esce
set -eu

TEMPLATE="/usr/share/nginx/html/assets/env.template.js"
TARGET="/usr/share/nginx/html/assets/env.js"

# Sostituisci i placeholder con le variabili d'ambiente correnti
# Nota: envsubst sostituisce solo variabili in formato $VAR
envsubst < "$TEMPLATE" > "$TARGET"

exec "$@"
