#!/usr/bin/env bash
#
# Automated regression test: in FrankenPHP worker mode, requests that run
# without a debug client must not stop later requests from hitting
# breakpoints. The engine caches the observer decision per function and the
# worker reuses op_arrays across requests, so a single un-debugged request used
# to blacklist every function it touched for the rest of the worker's life.
#
# Two scenarios, each against a fresh worker:
#
#   poisoned  N requests with no trigger, then requests with one — must break
#   control   a trigger request against an untouched worker — must break
#
# Run from the repo root:
#
#   tests/frankenphp/run-worker-tests.sh
#
# Environment overrides: IMAGE, HTTP_PORT, DBGP_PORT, POISON_REQUESTS,
# TRIGGER_REQUESTS, SKIP_BUILD=1 (reuse an already built image).

set -euo pipefail

IMAGE="${IMAGE:-php-debugger-frankenphp}"
HTTP_PORT="${HTTP_PORT:-8081}"
DBGP_PORT="${DBGP_PORT:-9004}"
POISON_REQUESTS="${POISON_REQUESTS:-200}"
TRIGGER_REQUESTS="${TRIGGER_REQUESTS:-5}"
CONTAINER="php-debugger-frankenphp-test"

repo_root="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$repo_root"

workdir="$(mktemp -d)"
listener_pid=""

cleanup() {
	[ -n "$listener_pid" ] && kill "$listener_pid" 2>/dev/null || true
	docker rm -f "$CONTAINER" >/dev/null 2>&1 || true
	rm -rf "$workdir"
}
trap cleanup EXIT

fail() {
	echo "FAIL: $*" >&2
	exit 1
}

python3 - "$DBGP_PORT" <<'EOF' || fail "port $DBGP_PORT is already in use — set DBGP_PORT to a free port (an IDE listening on 9003 is the usual culprit)"
import socket, sys
s = socket.socket()
try:
    s.bind(("0.0.0.0", int(sys.argv[1])))
except OSError:
    sys.exit(1)
finally:
    s.close()
EOF

if [ "${SKIP_BUILD:-0}" != "1" ]; then
	echo "==> building $IMAGE"
	docker build -q -f tests/frankenphp/Dockerfile -t "$IMAGE" . >/dev/null
fi

# Starts a fresh worker plus a fresh DBGp listener, so no state leaks between
# scenarios. Echoes the listener log path.
start_worker() {
	local log="$1"

	docker rm -f "$CONTAINER" >/dev/null 2>&1 || true
	[ -n "$listener_pid" ] && kill "$listener_pid" 2>/dev/null || true

	DBGP_PORT="$DBGP_PORT" python3 tests/frankenphp/dbgp_listener.py >"$log" 2>&1 &
	listener_pid=$!
	disown "$listener_pid" 2>/dev/null || true

	docker run -d --name "$CONTAINER" \
		-p "$HTTP_PORT:80" \
		--add-host=host.docker.internal:host-gateway \
		-e XDEBUG_CLIENT_HOST=host.docker.internal \
		-e XDEBUG_CLIENT_PORT="$DBGP_PORT" \
		"$IMAGE" >/dev/null

	local i
	for i in $(seq 1 60); do
		if curl -fsS -o /dev/null "http://localhost:$HTTP_PORT/" 2>/dev/null; then
			return 0
		fi
		sleep 1
	done

	docker logs "$CONTAINER" >&2 || true
	fail "worker did not answer on port $HTTP_PORT within 60s"
}

hits() {
	grep -c 'BREAKPOINT HIT' "$1" || true
}

stacks_ok() {
	grep -c 'STACK OK' "$1" || true
}

echo "==> scenario 1: $POISON_REQUESTS requests without a trigger, then $TRIGGER_REQUESTS with one"
start_worker "$workdir/poisoned.log"
for _ in $(seq 1 "$POISON_REQUESTS"); do
	curl -fsS -o /dev/null "http://localhost:$HTTP_PORT/"
done
for _ in $(seq 1 "$TRIGGER_REQUESTS"); do
	curl -fsS -o /dev/null "http://localhost:$HTTP_PORT/?XDEBUG_TRIGGER=1"
	sleep 0.3
done
sleep 2
poisoned_hits="$(hits "$workdir/poisoned.log")"
echo "    breakpoints hit: $poisoned_hits/$TRIGGER_REQUESTS"
[ "$poisoned_hits" -eq "$TRIGGER_REQUESTS" ] || {
	cat "$workdir/poisoned.log" >&2
	fail "breakpoints stopped firing after un-debugged requests"
}
poisoned_stacks="$(stacks_ok "$workdir/poisoned.log")"
echo "    stacks containing the caller: $poisoned_stacks/$TRIGGER_REQUESTS"
[ "$poisoned_stacks" -eq "$TRIGGER_REQUESTS" ] || {
	cat "$workdir/poisoned.log" >&2
	fail "the debugger stopped but could not see the enclosing function — the observer was blacklisted by the un-debugged requests"
}

echo "==> scenario 2: a trigger request against a fresh worker"
start_worker "$workdir/control.log"
curl -fsS -o /dev/null "http://localhost:$HTTP_PORT/?XDEBUG_TRIGGER=1"
sleep 2
control_hits="$(hits "$workdir/control.log")"
echo "    breakpoints hit: $control_hits/1"
[ "$control_hits" -eq 1 ] || {
	cat "$workdir/control.log" >&2
	fail "a plain trigger request did not hit the breakpoint"
}
[ "$(stacks_ok "$workdir/control.log")" -eq 1 ] || {
	cat "$workdir/control.log" >&2
	fail "a plain trigger request produced an incomplete stack"
}

echo "PASS: FrankenPHP worker-mode breakpoints survive un-debugged requests"
