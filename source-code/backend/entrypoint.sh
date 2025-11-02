#!/bin/sh

# -e: Se un comando fallisce, la shell esce
set -e

DB_FILE="./db/data/database.sqlite"

# Se il file non esiste o è vuoto
if [ ! -s "$DB_FILE" ]; then
    echo "Initializing the database..."
    mkdir -p "$(dirname "$DB_FILE")"
    sqlite3 "$DB_FILE" < ./db/scheme.sql
    sqlite3 "$DB_FILE" < ./db/populate_db.sql
    echo "Database successfully initialized"
else
    echo "Database already present, no initialization necessary."
fi

exec "$@"
