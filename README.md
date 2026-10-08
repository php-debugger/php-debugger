<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="assets/php-debugger-lockup-white.png">
    <img src="assets/php-debugger-lockup.png" alt="PHP Debugger — zero-overhead debugging" width="420">
  </picture>
</p>

PHP Debugger is a step debugger for PHP, and nothing else. Every other feature that
normally ships alongside one — the profiler, the coverage collector, the tracer — has
been left out. What remains is a debugger you can leave switched on permanently,
because when you are not using it you can barely tell it is there. It speaks the DBGp
protocol, so PhpStorm, VS Code and anything else that already debugs PHP works with it,
and it accepts Xdebug's INI settings, triggers and functions, so in most projects there
is nothing to migrate beyond the line that loads the extension.

📖 **Full documentation: [php-debugger.dev](https://php-debugger.dev)**

## Why PHP Debugger?

- **Near-zero overhead** when loaded with no debug client connected
- **Drop-in compatible** — existing INI settings, IDE configurations and helper functions keep working
- **Always on** — debugging starts with every request, so there is no trigger to remember
- **Compiled in, or not** — a PHP interpreter with the debugger built in, or a plain extension
- **Debug-only** — one job, done well, which is exactly why the rest of the time it costs so little

### Overhead

Measured in GitHub Actions with Valgrind instruction counts rather than wall-clock
timing, averaged across every supported PHP version, with the extension loaded and no
IDE connected — the state your machine is in almost all of the time.

| Benchmark                                     |      Xdebug | PHP Debugger |
|-----------------------------------------------|------------:|-------------:|
| `bench.php` — synthetic, computationally heavy | **+661.6%** |   **+12.9%** |
| Rector — a RectorPHP rule over a PHP file      | **+124.5%** |    **+3.6%** |
| Symfony — a basic request on a demo project    |  **+35.3%** |    **+1.3%** |

The synthetic benchmark is the worst case: tight loops of function calls and little
else, so the per-call cost has nowhere to hide. The closer you get to a real
application, the smaller the share of the work the debugger touches.

## Installation

**Requirements:** PHP 8.2, 8.3, 8.4, or 8.5.

### Installer

Get the installer — macOS and Linux:

```bash
curl -fsSL https://github.com/php-debugger/installer/releases/latest/download/install.sh | sh
```

Windows:

```powershell
powershell -c "irm https://github.com/php-debugger/installer/releases/latest/download/install.ps1 | iex"
```

Then install the debugger:

```bash
php-debugger install
```

That installs a self-contained PHP interpreter with the debugger compiled in and makes
it the active `php` on your PATH. To keep the PHP you already have and install only the
extension into it, use `php-debugger install --extension-only`. Either way, whatever it
replaced is backed up and `php-debugger uninstall` puts it back.

See [Installation](https://php-debugger.dev/getting-started/installation) for the full set of flags.

### Docker

The images on Docker Hub are drop-in replacements for the [official PHP images](https://hub.docker.com/_/php) — change one line:

```dockerfile
FROM phpdebugger/php:8.4-fpm
```

All official variants (`cli`, `fpm`, `apache`, `zts`, and their Alpine equivalents) for
PHP 8.2–8.5, on amd64 and arm64. Everything from the official images works unchanged,
including `docker-php-ext-install`. See [Docker](https://php-debugger.dev/getting-started/docker).

### Other options

PIE, prebuilt binaries and building from source are covered in
[More install options](https://php-debugger.dev/getting-started/install-options).

## IDE Setup

There is almost certainly nothing to change. PHP Debugger speaks the protocol your
editor already knows, on the port it already listens on — no plugin, no adapter, no new
configuration. Start listening on port `9003`, set a breakpoint, and run your code.

What changes is a habit rather than a setting: debugging is always available, so the
switch is your editor's listener, not the debugger's configuration. There is no
`?XDEBUG_SESSION_START=1` to remember and no browser extension to click.

See [IDE Support](https://php-debugger.dev/integrations/ide-support), or the guides for
[PhpStorm](https://php-debugger.dev/integrations/phpstorm) and
[VS Code](https://php-debugger.dev/integrations/vs-code).

## Xdebug Compatibility

| Feature                      | PHP Debugger    | Xdebug |
|------------------------------|-----------------|--------|
| Step debugging (DBGp)        | ✅               | ✅      |
| `xdebug.*` INI settings      | ✅ works         | ✅ works |
| `xdebug_break()`             | ✅ works         | ✅ works |
| `XDEBUG_SESSION` trigger     | ✅ works         | ✅ works |
| Code coverage                | ❌ use pcov      | ✅      |
| Profiling                    | ❌ removed       | ✅      |
| Tracing                      | ❌ removed       | ✅      |

## Documentation

Everything else — configuration, troubleshooting and the full reference — is at
**[php-debugger.dev](https://php-debugger.dev)**.

- [Quick start](https://php-debugger.dev/getting-started/quick-start)
- [User guide](https://php-debugger.dev/user-guide/starting-the-debugger) · [Troubleshooting](https://php-debugger.dev/user-guide/troubleshooting)
- [Settings](https://php-debugger.dev/reference/settings) · [Functions](https://php-debugger.dev/reference/functions) · [Environment variables](https://php-debugger.dev/reference/environment-variables)

## License

Released under [The Xdebug License](LICENSE), version 1.03 (based on The PHP License).

This product includes Xdebug software, freely available from [https://xdebug.org/](https://xdebug.org/).

## Acknowledgments

PHP Debugger is built on the foundation of [Xdebug](https://xdebug.org/), created and maintained by **Derick Rethans** since 2002. His two decades of work on PHP debugging tools made this project possible. Thank you, Derick.
