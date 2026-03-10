# PHP Debugger

A lightweight, high-performance PHP debugger extension. Forked from [Xdebug](https://xdebug.org/) by Derick Rethans, stripped down to pure debugging, and optimized for near-zero overhead when inactive.

## Why PHP Debugger?

- **~10% overhead** when loaded but inactive (vs ~630% before optimization)
- **Drop-in Xdebug replacement** — existing configs, IDE setups, and workflows work unchanged
- **Debug-only** — no profiler, no coverage, no tracing. Just debugging, done right.
- **Full DBGp protocol support** — works with PhpStorm, VS Code, and any DBGp-compatible IDE

### Benchmarks

| Configuration | Time | vs Vanilla |
|---|---|---|
| Vanilla PHP (no ext) | 0.139s | baseline |
| PHP Debugger, no trigger | 0.154s | ~1.1× |
| PHP Debugger, triggered, no client | 0.145s | ~1.04× |
| Xdebug, triggered, no client | 0.589s | ~4.2× |

*Apple Silicon, PHP 8.5.3, bench.php*

## Installation

### From source

```bash
git clone https://github.com/pronskiy/php-debugger-src.git
cd php-debugger-src
phpize
./configure --enable-php-debugger
make
sudo make install
```

Add to your `php.ini`:

```ini
zend_extension=php_debugger.so
```

### Pre-built binaries

Download from [Releases](https://github.com/pronskiy/php-debugger-src/releases) (Linux x64 + macOS ARM64, PHP 8.2–8.5).

## Configuration

PHP Debugger accepts both `php_debugger.*` and `xdebug.*` INI prefixes. Existing Xdebug configurations work as-is.

```ini
; Both of these work:
php_debugger.mode = debug
php_debugger.client_host = 127.0.0.1
php_debugger.client_port = 9003
php_debugger.start_with_request = trigger

; Xdebug-compatible (also works):
xdebug.mode = debug
xdebug.client_host = 127.0.0.1
xdebug.client_port = 9003
xdebug.start_with_request = trigger
```

## IDE Setup

### PhpStorm

No changes needed. PhpStorm connects via DBGp — the same protocol as Xdebug. Just swap the extension and your existing debug configurations work.

### VS Code

Use the [PHP Debug](https://marketplace.visualstudio.com/items?itemName=xdebug.php-debug) adapter. No changes needed — it speaks DBGp.

## Xdebug Compatibility

PHP Debugger is a drop-in replacement for Xdebug\'s debug mode:

| Feature | PHP Debugger | Xdebug |
|---|---|---|
| `extension_loaded("xdebug")` | ✅ true | ✅ true |
| `extension_loaded("php_debugger")` | ✅ true | ❌ false |
| `xdebug.*` INI settings | ✅ works | ✅ works |
| `xdebug_break()` | ✅ works | ✅ works |
| `XDEBUG_SESSION` trigger | ✅ works | ✅ works |
| Step debugging (DBGp) | ✅ | ✅ |
| Profiling | ❌ removed | ✅ |
| Code coverage | ❌ removed | ✅ |
| Tracing | ❌ removed | ✅ |

### New names (optional)

You can also use the new names — they work alongside the Xdebug ones:

- **INI:** `php_debugger.mode`, `php_debugger.client_host`, etc.
- **Functions:** `php_debugger_break()`, `php_debugger_info()`, `php_debugger_connect_to_client()`, `php_debugger_is_debugger_active()`, `php_debugger_notify()`
- **Triggers:** `PHP_DEBUGGER_SESSION`, `PHP_DEBUGGER_SESSION_START`, `PHP_DEBUGGER_TRIGGER`

## Requirements

- PHP 8.2, 8.3, 8.4, or 8.5

## License

Released under [The Xdebug License](LICENSE), version 1.03 (based on The PHP License).

This product includes Xdebug software, freely available from [https://xdebug.org/](https://xdebug.org/).

## Acknowledgments

PHP Debugger is built on the foundation of [Xdebug](https://xdebug.org/), created and maintained by **Derick Rethans** since 2002. His two decades of work on PHP debugging tools made this project possible. Thank you, Derick.
