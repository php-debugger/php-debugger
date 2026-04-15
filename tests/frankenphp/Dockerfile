# Builds php-debugger from the local checkout against FrankenPHP's bundled
# ZTS PHP, then runs it in worker mode so the new SAPI hooks can be exercised.
#
# Build context = repo root:
#   docker build -f tests/frankenphp/Dockerfile -t php-debugger-frankenphp .
#
# Run (host listens on 9003 for the IDE):
#   docker run --rm -p 8080:80 \
#     -e XDEBUG_CLIENT_HOST=host.docker.internal \
#     -e XDEBUG_CLIENT_PORT=9003 \
#     php-debugger-frankenphp

FROM dunglas/frankenphp:1-php8.4 AS build

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential autoconf pkg-config git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . /src

# `.dockerignore` at the repo root keeps stale host build artifacts
# (Makefile, *.lo, *.dep, ... with absolute host paths baked in) out of
# the context. `phpize --clean` here is a belt-and-braces reset in case
# anything slipped through.
RUN phpize --clean 2>/dev/null || true \
 && phpize \
 && ./configure --enable-php-debugger \
 && make -j"$(nproc)" \
 && cp modules/php_debugger.so "$(php-config --extension-dir)/"

# ---- runtime ----
FROM dunglas/frankenphp:1-php8.4

# Copy the whole versioned extensions dir so the .so lands at the exact
# path PHP's `extension_dir` points to.
COPY --from=build /usr/local/lib/php/extensions/ /usr/local/lib/php/extensions/

# Load as a Zend extension (not a regular extension).
# Host / port are overridable at `docker run` time via env vars (see below)
# — PHP INI does not support `${VAR:default}` bash-style fallback, so we
# substitute at container startup via a tiny entrypoint instead.
RUN { \
      echo 'zend_extension=php_debugger.so'; \
      echo 'xdebug.mode=debug'; \
      echo 'xdebug.start_with_request=trigger'; \
      echo 'xdebug.client_host=host.docker.internal'; \
      echo 'xdebug.client_port=9003'; \
      echo 'xdebug.discover_client_host=0'; \
      echo 'xdebug.log=/tmp/xdebug.log'; \
      echo 'xdebug.log_level=7'; \
      echo 'xdebug.idekey=PHPSTORM'; \
    } > /usr/local/etc/php/conf.d/zz-php-debugger.ini

# Rewrite host/port from env vars if provided before handing off to FrankenPHP.
COPY tests/frankenphp/entrypoint.sh /usr/local/bin/php-debugger-entrypoint.sh
RUN chmod +x /usr/local/bin/php-debugger-entrypoint.sh
ENTRYPOINT ["/usr/local/bin/php-debugger-entrypoint.sh"]
CMD ["frankenphp", "run", "--config", "/etc/frankenphp/Caddyfile"]

COPY tests/frankenphp/app/        /app/
COPY tests/frankenphp/Caddyfile   /etc/frankenphp/Caddyfile

EXPOSE 80
