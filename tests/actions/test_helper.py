"""Exercise the shipped helper protocol without creating any input device."""
import json
from pathlib import Path
import select
import subprocess
import sys
import time
import unittest

HELPER = Path(__file__).resolve().parents[2] / 'CasterSoundboard/actions/input_helper.py'
BOOT = '''import importlib.util, sys, json
spec = importlib.util.spec_from_file_location('helper', sys.argv[1])
m = importlib.util.module_from_spec(spec); spec.loader.exec_module(m)
class Fake:
    def code(self, key): return 47
    def event(self, key, value): print(json.dumps({'key': value}), file=sys.stderr, flush=True)
    def close(self): pass
m.Keyboard = Fake
sys.argv = ['helper', 'KEY_V']
m.main()
'''

class HelperTests(unittest.TestCase):
    def setUp(self):
        self.p = subprocess.Popen([sys.executable, '-B', '-c', BOOT, str(HELPER)],
                                  stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                  text=True)
        self.assertEqual(self.read(), 'ready')

    def tearDown(self):
        if self.p.poll() is None:
            self.p.stdin.close(); self.p.wait(timeout=3)
        for f in (self.p.stdin, self.p.stdout, self.p.stderr): f.close()

    def read(self):
        self.assertTrue(select.select([self.p.stdout], [], [], 3)[0])
        return json.loads(self.p.stdout.readline())['event']

    def command(self, **value):
        self.p.stdin.write(json.dumps(value)+'\n'); self.p.stdin.flush()

    def events(self):
        self.p.stdin.close(); self.p.wait(timeout=3)
        return [json.loads(line)['key'] for line in self.p.stderr if line.startswith('{')]

    def test_ptt_normal_and_reuse(self):
        for _ in range(2):
            self.command(op='open', mode='ptt'); self.assertEqual(self.read(), 'opened')
            self.command(op='close'); self.assertEqual(self.read(), 'closed')
        self.assertEqual(self.events(), [1,0,1,0])

    def test_mute_pair(self):
        self.command(op='open', mode='mute'); self.assertEqual(self.read(), 'opened')
        self.command(op='close'); self.assertEqual(self.read(), 'closed')
        self.assertEqual(self.events(), [1,0,1,0])

    def test_mute_disconnect_pairs_toggle(self):
        self.command(op='open', mode='mute'); self.assertEqual(self.read(), 'opened')
        self.assertEqual(self.events(), [1,0,1,0])

    def test_heartbeat_loss_releases(self):
        self.command(op='open', mode='ptt'); self.assertEqual(self.read(), 'opened')
        self.p.wait(timeout=6)
        self.assertEqual(self.events(), [1,0])

    def test_cancel_during_opening_tap(self):
        self.command(op='open', mode='mute')
        self.command(op='close'); self.assertEqual(self.read(), 'closed')
        self.assertEqual(self.events(), [1,0,1,0])

if __name__ == '__main__': unittest.main()
