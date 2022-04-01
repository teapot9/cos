#!/usr/bin/env python3

import os
import subprocess
import signal

def handler(*_):
    print("intercepted sigint")

signal.signal(signal.SIGINT, handler)

os.remove('debug.log')
vm = subprocess.Popen(['./boot', 'efi'])
subprocess.run(['./boot', 'egdb'])
vm.kill()

