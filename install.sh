#!/usr/bin/env bash
set -euo pipefail

REPO="pronskiy/php-debugger-src"
EXT_NAME="php_debugger"

# Detect PHP version
if ! command -v php &>/dev/null; then
    echo "Error: php not found in PATH" >&2
    exit 1
fi

PHP_VER=$(php -r "echo PHP_MAJOR_VERSION . '.' . PHP_MINOR_VERSION;")
OS=$(uname -s | tr '[:upper:]' '[:lower:]')
ARCH=$(uname -m)

# Map OS names
case "$OS" in
    darwin) OS_TAG="macos" ;;
    linux)  OS_TAG="ubuntu" ;;
    *)      echo "Error: unsupported OS: $OS" >&2; exit 1 ;;
esac

# Artifact name matches build.yml pattern
ARTIFACT="php-debugger-php${PHP_VER}-${OS_TAG}-${ARCH}"
EXT_DIR=$(php -r "echo ini_get('extension_dir');")
EXT_FILE="${EXT_NAME}.so"

echo "PHP version:     ${PHP_VER}"
echo "Platform:        ${OS_TAG}-${ARCH}"
echo "Extension dir:   ${EXT_DIR}"
echo ""

# Download from latest release
URL="https://github.com/${REPO}/releases/latest/download/${ARTIFACT}.so"
echo "Downloading ${URL}..."

TMP=$(mktemp)
trap "rm -f $TMP" EXIT

if command -v curl &>/dev/null; then
    HTTP_CODE=$(curl -fsSL -w "%{http_code}" -o "$TMP" "$URL" 2>/dev/null || true)
elif command -v wget &>/dev/null; then
    wget -q -O "$TMP" "$URL" && HTTP_CODE=200 || HTTP_CODE=404
else
    echo "Error: neither curl nor wget found" >&2
    exit 1
fi

if [ ! -s "$TMP" ] || [ "${HTTP_CODE:-000}" != "200" ]; then
    echo "Error: could not download binary for PHP ${PHP_VER} on ${OS_TAG}-${ARCH}" >&2
    echo "Check available releases: https://github.com/${REPO}/releases" >&2
    exit 1
fi

# Install
if [ -w "$EXT_DIR" ]; then
    cp "$TMP" "${EXT_DIR}/${EXT_FILE}"
    chmod 755 "${EXT_DIR}/${EXT_FILE}"
else
    echo "Extension dir not writable, using sudo..."
    sudo cp "$TMP" "${EXT_DIR}/${EXT_FILE}"
    sudo chmod 755 "${EXT_DIR}/${EXT_FILE}"
fi

echo ""
echo "Installed ${EXT_FILE} to ${EXT_DIR}/"
echo ""

# Check if already in php.ini
if php -r "exit(extension_loaded('php_debugger') ? 0 : 1);" 2>/dev/null; then
    echo "Extension is already loaded. Done!"
else
    INI_DIR=$(php -r "echo PHP_CONFIG_FILE_SCAN_DIR;" 2>/dev/null || true)
    echo "Add to your php.ini:"
    echo ""
    echo "    zend_extension=${EXT_FILE}"
    echo ""
    if [ -n "$INI_DIR" ] && [ -d "$INI_DIR" ]; then
        echo "Or create ${INI_DIR}/99-php_debugger.ini with that line."
    fi
fi
