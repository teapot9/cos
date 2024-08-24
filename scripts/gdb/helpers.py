import gdb
import re
import os.path
import time
import signal

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

CommandQQ()
CommandRbreakIf()
