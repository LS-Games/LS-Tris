set "COMPOSE_DIR=../../.devcontainer"

docker compose --profile prod ^
    -f %COMPOSE_DIR%/docker-compose.backend.yml ^
    -f %COMPOSE_DIR%/docker-compose.frontend.yml ^
    --project-name ls-tris-prod ^
    down
PAUSE