#!/usr/bin/env python3
"""Minimal DBGp listener to reproduce php-debugger issue #63.

On accept:
  1. Read init packet.
  2. breakpoint_set on lib.php line 4 (`$a = $iter * 2;`).
  3. run.
  4. Print any subsequent <response> packets until disconnect.

DBGp framing: <length>\0<xml>\0
"""
import socket
import sys
import threading
import time

HOST = "0.0.0.0"
PORT = 9003

def read_packet(sock):
    """Read one DBGp packet: length\\0xml\\0."""
    length_buf = b""
    while True:
        ch = sock.recv(1)
        if not ch:
            return None
        if ch == b"\x00":
            break
        length_buf += ch
    length = int(length_buf)
    data = b""
    while len(data) < length:
        chunk = sock.recv(length - len(data))
        if not chunk:
            return None
        data += chunk
    trailing = sock.recv(1)
    if trailing != b"\x00":
        print(f"!! expected null terminator got {trailing!r}", flush=True)
    return data.decode("utf-8", errors="replace")

def send(sock, cmd):
    sock.sendall(cmd.encode("utf-8") + b"\x00")
    print(f">>> {cmd}", flush=True)

def handle_one(conn, addr, label):
    print(f"[{label}] connected from {addr}", flush=True)
    init = read_packet(conn)
    print(f"<<< init: {init}", flush=True)

    txn = 1
    send(conn, f"breakpoint_set -i {txn} -t line -f file:///app/lib.php -n 4")
    txn += 1
    resp = read_packet(conn)
    print(f"<<< {resp}", flush=True)

    send(conn, f"run -i {txn}")
    txn += 1
    while True:
        resp = read_packet(conn)
        if resp is None:
            print(f"[{label}] disconnected", flush=True)
            return
        print(f"<<< {resp}", flush=True)
        if "status=\"break\"" in resp:
            print(f"!!! BREAKPOINT HIT in [{label}]", flush=True)
            send(conn, f"run -i {txn}")
            txn += 1
        elif "status=\"stopping\"" in resp or "status=\"stopped\"" in resp:
            send(conn, f"stop -i {txn}")
            txn += 1
            return

def main():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((HOST, PORT))
    s.listen(8)
    print(f"listening on {HOST}:{PORT}", flush=True)
    n = 0
    while True:
        conn, addr = s.accept()
        n += 1
        label = f"conn#{n}"
        t = threading.Thread(target=handle_one, args=(conn, addr, label), daemon=True)
        t.start()

if __name__ == "__main__":
    main()
