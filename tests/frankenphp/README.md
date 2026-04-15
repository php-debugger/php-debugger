# Manual FrankenPHP worker-mode test

Smoke-tests the SAPI activate/deactivate hooks added in `src/debugger/frankenphp.c`.

## Build

From the **repo root** (the build context must include the whole tree):

```bash
docker build -f tests/frankenphp/Dockerfile -t php-debugger-frankenphp .
```

## Run

Start your IDE listening on port **9003** for incoming DBGp connections.

* **PhpStorm:** *Run → Start Listening for PHP Debug Connections* (the
  little phone icon in the toolbar should turn green).
* **VS Code:** add a `launch.json` with `"type": "php"`, `"request":
  "launch"`, `"port": 9003`, `"pathMappings": { "/app":
  "${workspaceFolder}/tests/frankenphp/app" }`, then F5.

Then start the container. **Bind-mount the app dir** so PhpStorm can match
incoming breakpoint paths to your local files:

```bash
# macOS / Windows: host.docker.internal works out of the box.
docker run --rm -p 8080:80 \
  -v "$PWD/tests/frankenphp/app:/app" \
  php-debugger-frankenphp

# Linux: add the host-gateway alias.
docker run --rm -p 8080:80 \
  --add-host=host.docker.internal:host-gateway \
  -v "$PWD/tests/frankenphp/app:/app" \
  php-debugger-frankenphp

# Override host/port from the CLI:
docker run --rm -p 8080:80 \
  -e XDEBUG_CLIENT_HOST=192.168.1.42 \
  -e XDEBUG_CLIENT_PORT=9003 \
  -v "$PWD/tests/frankenphp/app:/app" \
  php-debugger-frankenphp
```

The `XDEBUG_CLIENT_HOST` / `XDEBUG_CLIENT_PORT` env vars are applied at
container startup by `entrypoint.sh` (PHP INI has no native env-var
fallback syntax).

## One-time PhpStorm setup (path mapping)

Without a path mapping, PhpStorm receives `breakpoint_set` for `/app/index.php`
but doesn't know it corresponds to `tests/frankenphp/app/index.php` in your
project. The IDE will warn:

> It may be caused by path mappings misconfiguration or not synchronized
> local and remote projects.

Configure it once:

1. *Settings → PHP → Servers → +*
2. **Name:** `localhost` (any name; it just needs to exist)
3. **Host:** `localhost` · **Port:** `8080` · **Debugger:** Xdebug
4. ✅ **Use path mappings**
5. In the file tree, find `tests/frankenphp/app` and set the
   **Absolute path on the server** to `/app`.
6. *Apply* / *OK*.

### Quick connection check that bypasses path mapping

To prove the DBGp connection itself is working before fighting path
mappings, toggle *Run → Break at first line in PHP scripts* and hit:

```bash
curl -b 'XDEBUG_SESSION=PHPSTORM' http://localhost:8080/
```

PhpStorm should pause on the opening `<?php` regardless of any path config.
If even that doesn't fire, the connection isn't being made — see the
**Troubleshooting** section.

## What to verify

Open `tests/frankenphp/app/index.php` in your IDE and put a breakpoint on
the `$pid = getmypid();` line.

### 1. No trigger ⇒ no pause

```bash
curl -s http://localhost:8080/
```

Returns immediately. The IDE must not pause. `tail -f /tmp/xdebug.log`
inside the container shows `Trigger value ... not found, so not activating`.

### 2. Trigger ⇒ pause

```bash
curl -s -b 'XDEBUG_SESSION=PHPSTORM' http://localhost:8080/
```

PhpStorm pauses on the breakpoint. Resume (F9) — request completes.

### 3. Same worker, multiple debug requests

```bash
for i in 1 2 3; do
  curl -s -b 'XDEBUG_SESSION=PHPSTORM' http://localhost:8080/
done
```

The `pid=` field in every response stays the same (one worker process), but
the IDE pauses on **every** request. This proves the per-request
`sapi_module.activate` reset is working — without it, only the first
request would be debuggable.

### 4. Breakpoint added between requests (the `xdebug_dbgp_poll_pending` test)

This is the scenario that caught the original bug:

1. Set a breakpoint on **line 4** (`$pid = getmypid();`).
2. `curl -b 'XDEBUG_SESSION=PHPSTORM' http://localhost:8080/` → pauses on
   line 4. Resume (F9).
3. **Move the breakpoint to line 5** (`$now = ...`) while the worker is
   idle. The IDE sends `breakpoint_remove` + `breakpoint_set` to the
   socket of the (now idle) worker thread — those commands sit queued.
4. `curl -b 'XDEBUG_SESSION=PHPSTORM' http://localhost:8080/`.

   The next `sapi_module.activate` calls `xdebug_dbgp_poll_pending()`,
   which drains the queued commands and registers the new breakpoint.
   PhpStorm pauses on line 5.

   **Without the poll, the IDE's queued `breakpoint_set` is silently
   ignored and the new breakpoint is never honored** — exactly the
   symptom this PR fixes.

### 5. Logs

```bash
docker exec $(docker ps -qf ancestor=php-debugger-frankenphp) \
  tail -f /tmp/xdebug.log
```

Per debug request expect to see:

```
[Step Debug] INFO: Connecting to configured address/port: host.docker.internal:9003.
[Step Debug] INFO: Connected to debugging client: ...
... (DBGp protocol messages) ...
Log closed at ...
```

A clean `Log closed` between requests confirms the per-request
deactivate hook tore the session down before the next activate set up a
fresh one.

## Cleanup test

`docker stop <container>` triggers PHP MSHUTDOWN, which calls
`xdebug_frankenphp_mshutdown()` to restore the original
`sapi_module.activate` / `deactivate` pointers. Clean exit (no segfault
in the container logs) means the restore worked.

## Troubleshooting

| Symptom | Cause | Fix |
|---|---|---|
| `curl` returns nothing / 308 redirect to `https://` | FrankenPHP using its default Caddyfile, not ours | Confirm `tests/frankenphp/Caddyfile` was copied to `/etc/frankenphp/Caddyfile` (not `/etc/caddy/Caddyfile`) |
| `Failed loading Zend extension 'php_debugger.so'` | `.so` not in versioned ext dir | Confirm `COPY --from=build /usr/local/lib/php/extensions/ ...` (whole dir, not flattened) |
| `make: *** No rule to make target '/Users/.../xdebug.c'` during build | Host build artifacts (`Makefile`, `*.dep`) leaked into context | Repo-root `.dockerignore` should exclude `**/*.dep`, `**/*.lo`, `Makefile`, etc. — check it exists |
| IDE warns about path mappings | Local file path ≠ container file path | Configure **Settings → PHP → Servers** as described above; bind-mount `-v "$PWD/tests/frankenphp/app:/app"` |
| `Connecting to ... :0` in xdebug log | Tried to use `${VAR:default}` in PHP INI, which doesn't expand | Already fixed via `entrypoint.sh`; pass real `XDEBUG_CLIENT_HOST` / `_PORT` env vars at `docker run` |
| `Creating socket ... error` in xdebug log | IDE not reachable from container | Linux: add `--add-host=host.docker.internal:host-gateway`. Or set `XDEBUG_CLIENT_HOST` to the host IP visible from the container. Verify the IDE is actually listening on the chosen port. |
