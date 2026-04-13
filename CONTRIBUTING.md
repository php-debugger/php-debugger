# Contributing to PHP Debugger

## Building

Prerequisites: PHP 8.2–8.5 with dev headers, phpize, autoconf, make, gcc/clang. Linux needs `linux-headers` for `rtnetlink.h`.

```bash
git clone https://github.com/php-debugger/php-debugger.git
cd php-debugger
phpize
./configure --enable-php-debugger
make
```

Dev build (extra warnings): `./configure --enable-php-debugger --enable-xdebug-dev`

Load as **Zend extension** (not `extension=`):

```ini
zend_extension=php_debugger.so
```

## Running Tests

```bash
php run-xdebug-tests.php tests/                              # Full suite
php run-xdebug-tests.php tests/debugger/                     # Subset
php run-xdebug-tests.php tests/debugger/bug01385.phpt        # Single
```

Coverage: `make test-coverage` / `make test-coverage-lcov`
C tests: `tests/ctest/` with own Makefile (Google Test)

## PR Conventions

- Branch off `main`, one issue = one PR
- Branch names: `fix/bug-1234`, `feature/dap-support`, `refactor/strip-tracing`
- Reference issue numbers: `Fix #123: handle null return in eval`
- Squash or rebase before merge — keep history clean, one logical commit per PR

### PR checklist

1. Tests pass: `php run-xdebug-tests.php tests/`
2. New tests added for behavioral changes
3. No unrelated changes
4. Clean build with `--enable-xdebug-dev`

## Code Style

- Tab indentation, braces on same line
- Functions: `xdebug_` prefix, `snake_case`
- Types: `xdebug_*_t` suffix
- Macros: `XDEBUG_`, `XG_`, `XINI_` prefixes
- Global accessors: `XG_DBG(field)`, `XG_BASE(field)`
- INI accessors: `XINI_DBG(field)`, `XINI_BASE(field)`
- Headers guarded: `#ifndef __XDEBUG_<MODULE>_H__`
- Every file has the Xdebug license block
