# Author: Artem Nefedov

import gdb
import re
import os.path
import time
import signal

TIMEOUT = 10
EFI = 'build/arch/x86_64/boot/kernel.so'
EFI_DEBUG = EFI
KERNEL = 'build/kernel.elf'
DEBUG_LOG = '/tmp/qemu-efi/debug.log'

CMD_FILE = 'auto.gdb'

class CommandEfi(gdb.Command):
    def __init__(self):
        super().__init__('efi', gdb.COMMAND_OBSCURE)

    def invoke(self, arg, from_tty):
        #gdb.execute('file')
        #gdb.execute('symbol-file')
        #gdb.execute('set architecture i386:x86-64:intel')
        #gdb.execute('set disassembly-flavor intel')

        #pagination = gdb.execute('show pagination', to_string=True).split()[-1].rstrip('.')
        #if pagination:
        #    gdb.execute('set pagination off')

        loading = r'Loading [^ ]+ at (0x[0-9A-F]{8,}) .*'
        bootx64 = r'FSOpen: Open .*BOOTX64\.EFI.*'
        klog = r'efistub: kernel loaded at 0x([0-9A-Fa-f]{4,16})\n$'
        klog2 = r'Kernel loaded at 0x([0-9A-Fa-f]{4,16})\n$'
        base = None
        start = time.time()
        while not os.path.isfile(DEBUG_LOG) \
                and TIMEOUT >= time.time() - start:
            time.sleep(1)
        while base is None and TIMEOUT >= time.time() - start:
            with open(DEBUG_LOG) as f:
                matched = False
                for l in f:
                    if not matched:
                        if re.match(klog2, l) or re.match(klog, l) or re.match(bootx64, l):
                            matched = True
                        else:
                            continue
                    m = re.match(loading, l)
                    k = re.match(klog, l)
                    k2 = re.match(klog2, l)
                    if m:
                        print(f"base found with {loading}")
                        base = int(m.group(1), base=16)
                        break
                    if k:
                        print(f"base found with {klog}")
                        base = int(k.group(1), base=16)
                        break
                    if k2:
                        print(f"base found with {klog2}")
                        base = int(k2.group(1), base=16)
                        break
            if base is None:
                time.sleep(0.001)
        if base is None:
            print("Base not found")
            return
        print(f"Base: 0x{base:x}")

        gdb.execute(f'file {EFI}')
        sections = {}
        section_reg = r'^\s*0x.* is \.[a-z.]+$'
        for l in gdb.execute('info files', to_string=True).split('\n'):
            if re.match(section_reg, l):
                sections[l.split()[-1]] = int(l.split()[0], base=16)
        gdb.execute('file')
        if not sections:
            print("No section found")
            return
        print(f"Found sections: {sections}")

        s = f'add-symbol-file {EFI_DEBUG} 0x{base+sections[".text"]:X}'
        for section in sections:
            if section != '.text':
                s += f' -s {section} 0x{base + sections[section]:X}'
        print(f"Running {s}")
        gdb.execute(s)

        #if pagination:
        #    gdb.execute('set pagination on')
        gdb.execute('target remote :1234')

        gdb.execute(f'add-symbol-file {KERNEL}')

        '''
        try:
            with open(CMD_FILE, 'r') as cmd_file:
                for cmd in cmd_file:
                    if not cmd.startswith('#') and cmd.strip():
                        print(f"uefi.py: {repr(cmd)}")
                        if cmd.startswith('@'):
                            silent = True
                            cmd = cmd.removeprefix('@')
                        else:
                            silent = False
                        if cmd.startswith('*'):
                            cmd = cmd.removeprefix('*')
                            break_ = gdb.breakpoints()[-1]
                            oldcmd = break_.commands or ''
                            break_.commands = oldcmd + cmd
                            continue
                        gdb.execute(cmd, to_string=silent)
        except OSError:
            gdb.execute('break entry_efi_s1')
            gdb.execute('break entry_efi_s2')
            gdb.execute('continue')
        print([b.number for b in gdb.breakpoints()])
        '''

CommandEfi()
