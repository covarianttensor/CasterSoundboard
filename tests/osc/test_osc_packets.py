#!/usr/bin/env python3
"""Send OSC listener regression cases to a running local CasterSoundboard.

Usage: python3 test_osc_packets.py [host] [port]
Defaults: 127.0.0.1:5051 (set OSC listener to this port in the app).
A process crash is detected manually: keep the GUI open and send a valid OSC
command afterwards to confirm the app still responds.
"""
import socket
import struct
import sys
import time

host = sys.argv[1] if len(sys.argv) > 1 else '127.0.0.1'
port = int(sys.argv[2]) if len(sys.argv) > 2 else 5051


def padded(data: bytes) -> bytes:
    return data + b'\x00' * ((-len(data)) % 4)


def osc(address: str, tags: str, args: bytes = b'') -> bytes:
    return padded(address.encode() + b'\x00') + padded(b',' + tags.encode() + b'\x00') + args

# Messages target unknown addresses, except for a sync request without the
# required argument. None should modify your soundboard state.
cases = [
    ('empty packet', b''),
    ('random bytes', b'garbage!'),
    ('unterminated address', b'/abcdefg'),
    ('missing tags', padded(b'/unknown\x00')),
    ('malformed type tag', osc('/unknown', 'z')),
    ('truncated int', osc('/unknown', 'i')),
    ('truncated float', osc('/unknown', 'f')),
    ('truncated long', osc('/unknown', 'h')),
    ('unterminated string', padded(b'/unknown\x00') + b',s\x00\x00' + b'xxxx'),
    ('oversized blob length', osc('/unknown', 'b', struct.pack('>I', 0x7FFFFFFF))),
    ('oversized packet', b'/' + b'A' * 8195),
    ('negative blob length', osc('/unknown', 'b', struct.pack('>i', -1))),
    ('unclosed array', osc('/unknown', '[')),
    ('reversed array brackets', osc('/unknown', '][')),
    ('no arguments on command address', osc('/glo/sync', '')),
    ('ignored bundle', b'#bundle\x00' + b'\x00' * 8),
    ('empty blob followed by integer', osc('/unknown', 'bi', struct.pack('>ii', 0, 42))),
    ('valid integer message, unknown address', osc('/unknown', 'i', struct.pack('>i', 1))),
    ('valid float message, unknown address', osc('/unknown', 'f', struct.pack('>f', 0.5))),
]

with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
    for name, payload in cases:
        s.sendto(payload, (host, port))
        print(f'Sent {name} ({len(payload)} bytes)')
        time.sleep(0.12)
print('Check that CasterSoundboard is still running and accepts ordinary OSC commands.')
