#!/bin/sh
# Patch xdebug.client_host / xdebug.client_port from env vars before FrankenPHP
# starts. PHP INI has no native ${VAR:default} fallback, so we do it here.
set -e

INI=/usr/local/etc/php/conf.d/zz-php-debugger.ini

if [ -n "$XDEBUG_CLIENT_HOST" ]; then
    sed -i "s|^xdebug.client_host=.*|xdebug.client_host=${XDEBUG_CLIENT_HOST}|" "$INI"
fi
if [ -n "$XDEBUG_CLIENT_PORT" ]; then
    sed -i "s|^xdebug.client_port=.*|xdebug.client_port=${XDEBUG_CLIENT_PORT}|" "$INI"
fi

exec docker-php-entrypoint "$@"
