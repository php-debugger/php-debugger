# PHP Debugger Docker Images

Drop-in replacements for the [official PHP images](https://hub.docker.com/_/php) with [PHP Debugger](https://github.com/php-debugger/php-debugger) statically compiled into the interpreter. Change one line and you have a debug-ready container with near-zero overhead when no debug client is connected:

```dockerfile
# before
FROM php:8.4-fpm

# after
FROM phpdebugger/php:8.4-fpm
```

No `pecl install`, no `docker-php-ext-enable`, no separate dev Dockerfile. The debugger is always there and costs almost nothing until an IDE connects.

## ⚠️ Development use only

These images are meant for development environments and should **not** be used in production. A debugger gives whoever connects to it full access to your source code, variables and runtime data — with debugging active by default, a reachable production container is a serious security risk. Keep your production images on the official `php:` base and use these only where the debugger is wanted.

## Supported tags

Images are built for PHP **8.2, 8.3, 8.4 and 8.5**, for `linux/amd64` and `linux/arm64`, mirroring the official image variants:

| Tag | Distro |
|---|---|
| `8.x-cli` (also `8.x`) | Debian |
| `8.x-fpm` | Debian |
| `8.x-apache` | Debian |
| `8.x-zts` | Debian |
| `8.x-cli-alpine` (also `8.x-alpine`) | Alpine |
| `8.x-fpm-alpine` | Alpine |
| `8.x-zts-alpine` | Alpine |
| `latest` | newest stable PHP, cli variant |

Each tag matches the official `php:` tag of the same name.

There are no patch-level tags (`8.4.23`): each tag always contains the latest patch release of its PHP minor version. Images are rebuilt on every PHP Debugger release and weekly, so they track new PHP patch releases within a few days.

## Usage

Everything from the official images works unchanged — same entrypoints, same helper scripts, same config layout:

```dockerfile
FROM phpdebugger/php:8.4-fpm

RUN docker-php-ext-install -j$(nproc) pdo_mysql bcmath
COPY --from=composer:2 /usr/bin/composer /usr/bin/composer
```

The only difference from the official image: PHP Debugger is compiled in (as a static extension — it does not appear in `conf.d/` and cannot be uninstalled), and opcache's JIT compiler is disabled, since it is incompatible with the debugger's engine hooks.

> [!WARNING]
> The JIT compiler is disabled only in the opcache build that ships with the image. If you rebuild opcache yourself (`docker-php-ext-install opcache`), pass `--disable-opcache-jit` to `docker-php-ext-configure` first, or JIT will be compiled back in:
> ```dockerfile
> RUN docker-php-ext-configure opcache --disable-opcache-jit \
>     && docker-php-ext-install -j$(nproc) opcache
> ```

### Debugging

The debugger speaks the DBGp protocol, so PhpStorm, VS Code and any Xdebug-compatible client work as-is (the extension identifies itself as `xdebug` for compatibility). Typical development setup in `docker-compose.yml`:

```yaml
services:
    app:
        image: phpdebugger/php:8.4-fpm
        environment:
            XDEBUG_CONFIG: "client_host=host.docker.internal"
            PHP_IDE_CONFIG: "serverName=myapp"
        extra_hosts:
            - "host.docker.internal:host-gateway"   # needed on Linux
```

No INI configuration is needed: the debugger is active by default and connects to your IDE whenever it is listening, at near-zero cost when it isn't. If you do need to change settings, both `xdebug.*` and `php_debugger.*` INI prefixes are accepted.

**Migrating from an Xdebug-based image?** Remove your old Xdebug configuration — with this image the defaults do the right thing:

- remove the line that loads the extension (`zend_extension=xdebug.so` or the `docker-php-ext-enable xdebug` step) — the debugger is compiled in;
- remove any line that sets `xdebug.mode` — debugging is on by default;
- remove any line that sets `xdebug.start_with_request` — the debugger starts with every request by default.

### Verifying

```console
$ docker run --rm phpdebugger/php:8.4-cli php -v
PHP 8.4.x (cli) ... (NTS)
    with Zend OPcache ...
    with PHP Debugger vX.Y.Z, Copyright (c) 2002-2026, by Derick Rethans
```

## License

PHP Debugger is licensed under [The Xdebug License, version 1.03](https://github.com/php-debugger/php-debugger/blob/main/LICENSE). PHP itself is distributed under the [PHP License](https://www.php.net/license/).
