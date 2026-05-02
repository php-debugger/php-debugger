# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

PHP Debugger — a Zend extension forked from Xdebug, stripped to step-debugging only (DBGp). Profiling, coverage, tracing, and gcstats were removed. Goal: near-zero overhead when no debug client is attached, while remaining drop-in compatible with Xdebug INI/functions/triggers.

Supports PHP 8.2–8.5. The shared library is `modules/php_debugger.so` (`php_debugger.dll` on Windows). The Zend module name internally is still `xdebug` for compatibility (`extension_loaded("xdebug")` returns true).

## Build

Standard PECL/PHP-extension build flow:

```bash
phpize
./configure --enable-php-debugger
make -j$(nproc 2>/dev/null || sysctl -n hw.logicalcpu)
```

Developer flags (`-Werror`, ASAN, full warnings) — `./configure --enable-php-debugger --enable-xdebug-dev`.

`./configure` errors out if PHP version is `< 8.0` or `>= 8.7`. Configure script enforces it via `PHP_XDEBUG_FOUND_VERNUM` against `php-config --vernum`.

Load extension: `php -d zend_extension=$PWD/modules/php_debugger.so ...`. **Always `zend_extension=`, not `extension=`** — registration as a Zend extension is required (observer API, opcode handlers).

Linux requires `linux/rtnetlink.h` (Alpine: `apk add linux-headers`).

## Tests

`run-xdebug-tests.php` is the test runner — a customized PHP-source `run-tests.php` that knows about DBGp `.phpt` plumbing. Do NOT use the in-tree `run-tests.php` for anything (it's the upstream PHP one, kept around but unused for this project).

Run all tests:

```bash
TEST_PHP_EXECUTABLE=$(make findphp)
TEST_PHP_ARGS="-n -d session.save_path=/tmp -d zend_extension=$PWD/modules/php_debugger.so" \
  $TEST_PHP_EXECUTABLE -n run-xdebug-tests.php -j$(nproc) -q -x --show-diff
```

Single test or directory:

```bash
php -n run-xdebug-tests.php -q tests/debugger/bug00530.phpt
php -n run-xdebug-tests.php -q tests/debugger/
```

Useful flags: `-x` skips slow tests (sets `SKIP_SLOW_TESTS`), `-q` no-interaction, `--show-diff` prints `.diff` on failure, `--asan` adjusts expectations under sanitizers. `tests/utils.inc::check_reqs()` interprets `--SKIPIF--` directives like `dbgp`, `slow`, `opcache`, `!opcache`, `linux`, `osx`, `win`, `PHP >= 8.x`, `ext-xxx`, `unparallel`.

After failure, generated artifacts (`*.diff`, `*.log`, `*.out`, `*.exp`, `*.php`, `*.sh`) sit alongside the `.phpt`. They're gitignored — clean by removing or rerunning.

Test areas: `tests/debugger/` (DBGp protocol, the bulk), `tests/base/` (engine integration, INI/mode), `tests/library/` (helpers like `xdebug_info`), `tests/ctest/` (CppUTest-based C unit tests for the path-map parser, separate `make` in that dir, requires AFL for fuzz targets).

DBGp tests boot a `DebugClient` (`tests/debugger/dbgp/dbgpclient.php`) that opens a server socket, spawns PHP with the extension, and drives the `.phpt` script through DBGp commands. `tests/debugger/dbgp/dbgp-emulator.php` is a passive emulator for inspection.

Sanitizer build (see `.github/workflows/sanitizers.yml`): clang + `-fsanitize=address,undefined -fno-sanitize=function` against a debug PHP built with `--enable-address-sanitizer --enable-undefined-sanitizer`. The `-fno-sanitize=function` is required — UBSan's function-type check trips on PHP's internal cast patterns.

## Architecture

The code is organized into three layers under `src/`, plus the entry point. Reading order for newcomers: `xdebug.c` → `src/lib/lib.h` → `src/base/` → `src/debugger/`.

### `xdebug.c` — module entry & INI

Owns `PHP_MINIT/MSHUTDOWN/RINIT/RSHUTDOWN`, the `zend_module_entry`, and *all* INI directives. Two parallel INI tables share the same backing storage:

- Canonical `xdebug.*` entries (PHP_INI_BEGIN/END block).
- `php_debugger.*` aliases registered via `php_debugger_ini_entries[]`.

The aliases use wrapper handlers (`PHP_DEBUGGER_INI_WRAPPER`) that delegate to the canonical `OnUpdate*` and then call `php_debugger_sync_canonical()` so `ini_get('xdebug.x')` and `ini_get('php_debugger.x')` always agree. The wrapper deliberately ignores defaults (`php_debugger_ini_is_explicitly_set`) so writing a `php_debugger.*` value doesn't clobber an existing `xdebug.*` value. When changing INI surface area, add the directive to both tables.

Mode dispatch is centralized: nearly every lifecycle hook starts with `if (XDEBUG_MODE_IS_OFF()) return;` or `if (XDEBUG_MODE_IS(XDEBUG_MODE_STEP_DEBUG))`. `XDEBUG_MODE_OFF` and `XDEBUG_MODE_STEP_DEBUG` are the only modes left after the fork — non-debug modes from upstream Xdebug were removed but the bitmask scheme remains so future modes slot in cleanly.

Global state lives in `zend_xdebug_globals` (defined in `php_xdebug.h`): two parallel sub-structs `globals.{base,debugger,library}` for runtime state and `settings.{base,debugger,library}` for INI-bound values. Access via `XG()`, `XG_BASE()`, `XINI_BASE()` macros (see also `XG_DBG`, `XG_LIB`, `XINI_LIB`, `XINI_DBG` in the per-component headers).

### `src/lib/` — shared utilities, INI/mode logic, var dumping

- `lib.c/h` — mode bitmask, `start_with_request`, control-socket granularity, opcode-handler multiplexing, active-frame tracking, output dirs, var-display limits. The `xdebug_global_mode` int and `xdebug_lib_set_mode()` are the single source of truth for mode.
- `var.c`, `var_export_{html,line,text,xml}.c` — zval inspection and serialization (DBGp uses XML).
- `maps/` — path-mapping mini-language and parser (used by `xdebug.path_mapping`); has its own C unit tests in `tests/ctest/`.
- `compat.c/h` — PHP-version compatibility shims for engine API drift across 8.2–8.5.
- `compat_stubs.c/h` — empty/stub implementations of Xdebug functions that were stripped (profiler, coverage, trace, gcstats, develop helpers like `xdebug_var_dump`). They exist so existing code calling `xdebug_get_code_coverage()`, etc., doesn't fatal. When deciding whether a function should be a real impl or a stub, check whether it appears in `php_xdebug.stub.php` / `php_xdebug_arginfo.h` (regenerated from the stub via `phpize` + `gen_stub.php`).
- `usefulstuff.c`, `cmd_parser.c`, `xml.c`, `str.c`, `hash.c`, `llist.c`, `set.c`, `vector.h`, `crc32.c`, `timing.c`, `log.c`, `headers.c`, `file.c`, `normalize_path.c`, `trim.c`, `xdebug_strndup.c`.

### `src/base/` — Zend engine integration

- `base.c` — observer/opcode/error/exception/throw/compile-file callbacks the extension installs into the Zend engine. This is the hot path; functions here run on every PHP statement when debugging is active.
- `filter.c` — code filter (path/class allow/deny lists used to skip tracing/breaks in vendor code).
- `ctrl_socket.c` — netlink-based control socket on Linux (gated by `HAVE_XDEBUG_CONTROL_SOCKET_SUPPORT`).

The "early connection" path (`xdebug.c` RINIT, `xg->early_connection`) attempts to attach the DBGp client at request init so the observer-based statement handler can decide whether to install itself for the request. When no client is reachable and `on_demand_debugging_enabled=0`, the statement handler stays uninstalled — this is the optimization that delivers the "near-zero overhead" claim. Setting `on_demand_debugging_enabled=1` keeps it installed regardless, supporting `xdebug_break()` / late `xdebug_connect_to_client()`.

### `src/debugger/` — DBGp protocol

- `debugger.c` — orchestration: when to break, breakpoint resolution, error/exception hooks, status machine (`STATUS_*`), eval support.
- `com.c` — TCP transport, connection setup, optional zlib compression, IDE key, host discovery (`discover_client_host`).
- `handler_dbgp.c` — the DBGp command dispatcher (`run`, `step_into`, `breakpoint_set`, `property_get`, `context_get`, `eval`, `feature_get`, etc.). The single biggest file; one handler function per command.
- `handlers.c` — generic handler-table glue.
- `ip_info.c` — interface info for client discovery.

Wire format is XML (see `src/lib/xml.c`); responses use `xdebug_xml_node` trees serialized at send time.

## INI / function naming

User-visible names exist in two parallel namespaces — `xdebug.*` / `xdebug_*()` (canonical, kept for compatibility) and `php_debugger.*` / `php_debugger_*()` (new). The implementation always uses the `xdebug_` C symbol; the `php_debugger_*` PHP-visible functions are thin aliases registered alongside (see `php_xdebug.stub.php` and the arginfo file). When adding a new public function, register both names.

INI source-of-truth is the `xdebug.*` entry; the alias only exists to let users write `php_debugger.foo` in their `php.ini`. `XDEBUG_CONFIG` and `PHP_DEBUGGER_CONFIG` env vars are both honored (`xdebug_env_config` in `xdebug.c`).

## Stub regeneration

`php_xdebug.stub.php` is the source of truth for function signatures; `php_xdebug_arginfo.h` is generated from it by PHP's `gen_stub.php` (run via `phpize`). Don't edit `php_xdebug_arginfo.h` by hand — change the stub and regenerate.
