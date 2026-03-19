# PHP Debugger

A lightweight, high-performance PHP debugger extension. Forked from [Xdebug](https://xdebug.org/) by Derick Rethans, stripped down to pure debugging, and optimized for near-zero overhead when inactive.

> [!NOTE]
> **🧪 This project is an experiment to show how fast the PHP community can move using AI tools.**  
> The entire extension — fork, strip, optimize, rename — was built in days, not months, using PhpStorm, Claude Code, OpenClaw. We're here to move fast, adapt to change, and prove that PHP tooling can be the best in the industry.

## Why PHP Debugger?

- **97% performance improvement over Xdebug** when loaded but inactive
- **Drop-in Xdebug replacement** — existing configs, IDE setups, and workflows work unchanged
- **Debug-only** — no profiler, no coverage, no tracing. Just debugging.
- **Full DBGp protocol support** — works with PhpStorm, VS Code, and any DBGp-compatible IDE

### Benchmarks

The following benchmarks were run in GitHub's CI (GitHub Actions) environment using a standard Ubuntu runner. 
The performance was measured using Valgrind to count the number of executed instructions.
This is much more precise and reproducible than timing execution runs. All measuremments were done
using all supported PHP versions (the number shown is the average), with the extension loaded and
no IDE connected.

We measured three different scenarios which we believe represent a good mix of typical PHP operations:

- `bench.php`: a syntetic benchmark that runs a number of computationally heavy functions

| Configuration |    Overhead |
|---------------|------------:|
| No debugger   |           — |
| Xdebug        | **+661.6%** |
| PHP Debugger  |  **+12.9%** |

Improvement: **98.0%**

- `Rector`: running a RectorPHP rule on a PHP file

| Configuration |    Overhead |
|---------------|------------:|
| No debugger   |           — |
| Xdebug        | **+124.5%** |
| PHP Debugger  |   **+3.6%** |

Improvement: **97.1%**

- `Symfony`: running a basic request on a Symfony demo project

| Configuration |   Overhead |
|---------------|-----------:|
| No debugger   |          — |
| Xdebug        | **+35.3%** |
| PHP Debugger  |  **+1.3%** |

Improvement: **96.1%**

Overall improvement: **97.1%**

## Installation

### Manual download

Grab the right binary from [Releases](https://github.com/pronskiy/php-debugger/releases), copy it to your extension directory, and add to `php.ini`:

```ini
zend_extension=php_debugger.so
```

### 🚧 Coming soon 

**Quick install script:**

```bash
curl -fsSL https://raw.githubusercontent.com/pronskiy/php-debugger/main/install.php | php
```

**PIE (PHP Installer for Extensions):**

```bash
pie install pronskiy/php-debugger
```

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

Works as-is. Just swap the extension and your existing debug configurations work.

[Configuring Debugger in PhpStorm](https://www.jetbrains.com/help/phpstorm/configuring-xdebug.html)

### VS Code

Works as-is. No changes needed.

## Xdebug Compatibility

PHP Debugger is a drop-in replacement for Xdebug\'s debug mode:

| Feature                            | PHP Debugger | Xdebug  |
|------------------------------------|--------------|---------|
| `extension_loaded("xdebug")`       | ✅ true       | ✅ true  |
| `extension_loaded("php_debugger")` | ✅ true       | ❌ false |
| `xdebug.*` INI settings            | ✅ works      | ✅ works |
| `xdebug_break()`                   | ✅ works      | ✅ works |
| `XDEBUG_SESSION` trigger           | ✅ works      | ✅ works |
| Step debugging (DBGp)              | ✅            | ✅       |
| Code coverage                      | ❌ use pcov   | ✅       |
| Profiling                          | ❌ removed    | ✅       |
| Tracing                            | ❌ removed    | ✅       |

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
