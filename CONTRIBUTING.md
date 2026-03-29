# Contributing to PHP Debugger

Thank you for your interest in contributing to PHP Debugger! This project is an experiment in rapid PHP tool development, and we welcome community involvement to keep PHP tooling at the top of the industry.

## PR Process
* Pull Requests Only: All changes must be submitted via Pull Requests. Direct pushes to the main branch are strictly prohibited.

* Review: Every PR requires a review before merging to ensure stability and performance.

* Focused Changes: One feature or fix per PR. If you find multiple things to fix, please submit them as separate Pull Requests.

* Diff Quality: Review your own diff before submitting. Every change should be intentional. Do not include tooling artifacts (.idea/, .vscode/, etc.) or reformat untouched code.

## Development Setup
### How to Build
To compile the extension from source, ensure you have the PHP development headers installed, then run the following commands:

```
phpize
./configure --enable-php-debugger
make
```
### Code Style
* Indentation: This project inherits its code style from Xdebug and uses tabs for indentation.

* Consistency: Maintain local consistency with existing files.

## Testing Requirements
We place a high priority on performance and stability.

New Features: Any PR introducing new functionality must include corresponding .phpt tests in the tests/ directory.

No Regressions: Your changes must not introduce new XFAIL (expected failure) regressions.

### How to Run Tests
You can run the test suite using the provided run-xdebug-tests.php script:

```
TEST_PHP_ARGS='-dzend_extension=modules/php_debugger.so' php run-xdebug-tests.php -q tests/debugger/
```
## Getting Started
Check our [Issue Tracker] (https://github.com/pronskiy/php-debugger/issues) for "good first issue" labels to start contributing.

## License
By contributing to this project, you agree that your contributions will be licensed under The Xdebug License. You can find the full text of the license here: [LICENSE](https://github.com/pronskiy/php-debugger/blob/main/LICENSE)
