# php-debugger Progress

## Phase 1: Fork & Strip

### Phase 1.1: Strip non-debugger modules ✅ DONE (2026-03-05)
- Commit: `4d8e7fa`
- Removed: coverage, profiler, tracing, develop, gcstats — sources + tests
- 666 files changed, ~34,800 lines deleted
- config.m4 cleaned, zlib/compression removed
- Default mode changed from "develop" to "debug"

### Phase 1.2: Deep cleanup ✅ DONE (2026-03-05)
- Commit: `ad57261`
- Removed all remaining references to deleted modules across:
  - lib.h/lib.c (mode defines, mode parsing, struct members)
  - log.c/log.h (phpinfo sections, channel defines)
  - filter.c/filter.h (coverage/tracing filter types)
  - base.c/base_globals.h (dead globals, profiler hooks)
  - handler_dbgp.c (profiler DBGp command)
  - var.c (tracing context save/restore)
- Zero remaining references to removed modules
- Builds and loads on PHP 8.6: `modules/xdebug.so` (1.4MB)

### Phase 1.3: Rename (TODO)
- Rename from xdebug to php-debugger
- Update all identifiers, macros, INI prefixes
- Update extension name, version, copyright

## Phase 2: DAP Protocol (TODO)
- Implement Debug Adapter Protocol alongside/replacing DBGp

## Phase 3: Performance (TODO)
- Benchmark and optimize debugger overhead

## Repository
- Source: `pronskiy/php-debugger-src` on GitHub
- Branch: `main`
- Build node: php-src-build (95.111.253.12)
