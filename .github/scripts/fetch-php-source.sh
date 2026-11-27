#!/bin/sh
# Downloads a PHP source tarball (as resolved by resolve-php-sources.py),
# checks its sha256 and unpacks it into a directory.
#
# Release tarballs ship the generated parser and scanner sources, so building
# from them needs no bison or re2c, and a phpize build needs no buildconf.
#
# Usage: fetch-php-source.sh <tarball url> <sha256> <destination dir>
set -eu

URL="$1"
SHA256="$2"
DEST="$3"

TARBALL="${TMPDIR:-/tmp}/php-src.tar.xz"
curl -fsSL --retry 3 -o "$TARBALL" "$URL"
if command -v sha256sum >/dev/null 2>&1; then
    echo "$SHA256  $TARBALL" | sha256sum -c -
else
    echo "$SHA256  $TARBALL" | shasum -a 256 -c -
fi
mkdir -p "$DEST"
tar -xJf "$TARBALL" --strip-components=1 -C "$DEST"
rm -f "$TARBALL"
