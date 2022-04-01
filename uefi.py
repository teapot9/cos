#!/usr/bin/env python3
# Author: Artem Nefedov

import gdb
import re
import os.path
import time

TIMEOUT = 10
EFI = 'cos.so'
DEBUG = 'cos.so'

PLIST_FUN = '''\
define plist
  set var $n = $arg0
  while $n
    print *$n
    set var $n = $n->next
  end
end
'''
CMD_FILE = 'auto.gdb'

class CommandEfi(gdb.Command):
    def __init__(self):
        super().__init__('efi', gdb.COMMAND_OBSCURE)

    def invoke(self, arg, from_tty):
        gdb.execute('file')
        gdb.execute('symbol-file')
        pagination = gdb.execute('show pagination', to_string=True).split()[-1].rstrip('.')
        if pagination:
            gdb.execute('set pagination off')
        gdb.execute('set architecture i386:x86-64:intel')
        gdb.execute('set disassembly-flavor intel')

        loading = r'Loading [^ ]+ at (0x[0-9A-F]{8,}) .*'
        bootx64 = r'FSOpen: Open .*BOOTX64\.EFI.*'
        klog = r'efistub: kernel loaded at 0x([0-9A-Fa-f]{4,16})\n$'
        klog2 = r'Kernel loaded at 0x([0-9A-Fa-f]{4,16})\n$'
        base = None
        start = time.time()
        while not os.path.isfile('debug.log') \
                and TIMEOUT >= time.time() - start:
            time.sleep(1)
        while base is None and TIMEOUT >= time.time() - start:
            with open('debug.log') as f:
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
                        base = int(m.group(1), base=16)
                        break
                    if k:
                        base = int(k.group(1), base=16)
                        break
                    if k2:
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
        '''
        text = None
        data = None
        bss = None
        for l in gdb.execute('info files', to_string=True).split('\n'):
            if ' is .text' in l:
                text = int(l.split()[0], base=16)
            elif ' is .data' in l:
                data = int(l.split()[0], base=16)
            elif ' is .bss' in l:
                bss = int(l.split()[0], base=16)
        '''
        gdb.execute('file')
        if not sections:
            print("No section found")
            return
        print(f"Found sections: {sections}")
        '''
        if text is None or data is None or bss is None:
            print("Missing text or data")
            return
        print(f"Sections: text = 0x{text:x}, data = 0x{data:x}, bss = 0x{bss:x}")
        '''

        s = f'add-symbol-file {DEBUG} 0x{base+sections[".text"]:X}'
        for section in sections:
            if section != '.text':
                s += f' -s {section} 0x{base + sections[section]:X}'
        print(f"Running {s}")
        gdb.execute(s)

        '''
        etext = base + text
        edata = base + data
        ebss = base + bss
        print(f"Effective addresses: text = 0x{etext:x}, data = 0x{edata:x}, bss = 0x{ebss:x}")
        gdb.execute(f'add-symbol-file {DEBUG} 0x{etext:X} -s .data 0x{edata:X} -s .bss 0x{ebss:X}')
        '''

        if pagination:
            gdb.execute('set pagination on')
        gdb.execute('target remote :1234')

        gdb.execute(PLIST_FUN)

        try:
            with open(CMD_FILE, 'r') as cmd_file:
                for cmd in cmd_file:
                    if not cmd.startswith('#'):
                        if cmd.startswith('@'):
                            silent = True
                            cmd = cmd.removeprefix('@')
                        else:
                            silent = False
                        gdb.execute(cmd, to_string=silent)
        except OSError:
            gdb.execute('break entry_efi')
            gdb.execute('continue')

class CommandQQ(gdb.Command):
    def __init__(self):
        super().__init__('qq', gdb.COMMAND_OBSCURE)

    def invoke(self, arg, from_tty):
        gdb.execute('set confirm off')
        gdb.execute('kill')
        gdb.execute('quit')

class CommandRbreakIf(gdb.Command):
    def __init__(self):
        super().__init__('rbreakif', gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        rarg = r'^([^ ]*)( if )?(.*)?$'
        name = re.match(rarg, arg).group(1)
        cond = re.match(rarg, arg).group(3)
        breaks = gdb.rbreak(name)
        if cond:
            for b in breaks:
                b.condition = cond

CommandEfi()
CommandQQ()
CommandRbreakIf()
