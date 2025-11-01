#!/bin/bash
COMPOSE_DIR="../../.devcontainer"

docker compose --profile dev \
    -f "$COMPOSE_DIR/docker-compose.backend.yml" \
    -f "$COMPOSE_DIR/docker-compose.frontend.yml" \
    -f "$COMPOSE_DIR/docker-compose.dev.yml" \
    --project-name ls-tris-dev \
    down