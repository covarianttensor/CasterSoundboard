#!/usr/bin/env python3
"""Caster input helper: fixed key, bounded sessions, cleanup on pipe loss."""
import ctypes as C
import ctypes.util
import json
import os
import select
import signal
import sys
import time

class Keyboard:
    def __init__(self):
        self.device = C.c_void_p()
        self.ui = C.c_void_p()
        self.lib = C.CDLL(ctypes.util.find_library('evdev') or 'libevdev.so.2')
        signatures = {
            'libevdev_new': ([], C.c_void_p),
            'libevdev_free': ([C.c_void_p], None),
            'libevdev_set_name': ([C.c_void_p, C.c_char_p], None),
            'libevdev_enable_event_code': ([C.c_void_p, C.c_uint, C.c_uint, C.c_void_p], C.c_int),
            'libevdev_uinput_create_from_device': ([C.c_void_p, C.c_int, C.POINTER(C.c_void_p)], C.c_int),
            'libevdev_uinput_write_event': ([C.c_void_p, C.c_uint, C.c_uint, C.c_int], C.c_int),
            'libevdev_uinput_get_devnode': ([C.c_void_p], C.c_char_p),
            'libevdev_uinput_destroy': ([C.c_void_p], None),
            'libevdev_event_code_from_name': ([C.c_uint, C.c_char_p], C.c_int),
        }
        for name, (args, result) in signatures.items():
            fn = getattr(self.lib, name)
            fn.argtypes, fn.restype = args, result
        self.device = self.lib.libevdev_new()
        if not self.device:
            raise MemoryError('libevdev_new')
        try:
            self.lib.libevdev_set_name(self.device, b'Caster uinput POC keyboard')
            # Advertise a normal keyboard for udev classification, but the helper
            # accepts commands for only the key fixed at startup. No physical input reads.
            for key in range(1, 249):
                self.check(self.lib.libevdev_enable_event_code(self.device, 1, key, None))
            self.check(self.lib.libevdev_uinput_create_from_device(self.device, -2, C.byref(self.ui)))
        except BaseException:
            self.close()
            raise

    @staticmethod
    def check(result):
        if result < 0:
            raise OSError(-result, os.strerror(-result))

    def code(self, name):
        value = self.lib.libevdev_event_code_from_name(1, name.encode('ascii'))
        if not 1 <= value < 249:
            raise ValueError('Use a standard Linux KEY_* keyboard code, e.g. KEY_V')
        return value

    def event(self, key, value):
        self.check(self.lib.libevdev_uinput_write_event(self.ui, 1, key, value))
        self.check(self.lib.libevdev_uinput_write_event(self.ui, 0, 0, 0))

    def node(self):
        return self.lib.libevdev_uinput_get_devnode(self.ui).decode()

    def close(self):
        if self.ui:
            self.lib.libevdev_uinput_destroy(self.ui)
            self.ui = C.c_void_p()
        if self.device:
            self.lib.libevdev_free(self.device)
            self.device = C.c_void_p()



def main():
    keyboard = Keyboard()
    key = keyboard.code(sys.argv[1])
    down = False
    mode = None
    tap_until = 0
    closing = False
    alive = True
    lease = time.monotonic() + 5
    maximum = 0
    pending = b''

    def emit(event):
        print(json.dumps({'event': event}), flush=True)

    def press():
        nonlocal down
        down = True
        keyboard.event(key, 1)

    def release():
        nonlocal down
        if down:
            keyboard.event(key, 0)
            down = False

    def close_session():
        nonlocal mode, tap_until, closing
        if mode == 'mute':
            release()
            press()
            tap_until = time.monotonic() + .08
            closing = True
        else:
            release()
            mode = None
            emit('closed')

    def stop(*_):
        nonlocal alive
        alive = False

    signal.signal(signal.SIGTERM, stop)
    signal.signal(signal.SIGINT, stop)
    try:
        time.sleep(1.5)
        emit('ready')
        while alive:
            now = time.monotonic()
            if now >= lease or (mode and now >= maximum):
                raise RuntimeError('Keyboard session expired')
            if tap_until and now >= tap_until:
                release()
                tap_until = 0
                if closing:
                    mode = None
                    closing = False
                    emit('closed')
                else:
                    emit('opened')
            if not select.select([sys.stdin.fileno()], [], [], .02)[0]:
                continue
            chunk = os.read(sys.stdin.fileno(), 4096)
            if not chunk:
                break
            pending += chunk
            if len(pending) > 4096:
                raise ValueError('Oversized command')
            while b'\n' in pending:
                line, pending = pending.split(b'\n', 1)
                command = json.loads(line)
                op = command.get('op')
                if op == 'ping':
                    lease = time.monotonic() + 5
                elif op == 'open' and mode is None:
                    requested = command.get('mode')
                    if requested not in ('ptt', 'mute'):
                        raise ValueError('Invalid talk mode')
                    mode = requested
                    maximum = time.monotonic() + 305
                    press()
                    if mode == 'mute':
                        tap_until = time.monotonic() + .08
                    else:
                        emit('opened')
                elif op == 'close' and mode and not closing:
                    close_session()
                else:
                    raise ValueError('Invalid command/state')
    finally:
        try:
            release()
            # A mute toggle cannot restore a known game state. Pair the opening
            # tap on orderly disconnect/expiry unless a closing tap was issued.
            if mode == 'mute' and not closing:
                press()
                time.sleep(.08)
                release()
        finally:
            keyboard.close()

if __name__ == '__main__':
    try:
        main()
    except (OSError, RuntimeError, ValueError) as exc:
        print(str(exc), file=sys.stderr, flush=True)
        sys.exit(1)
