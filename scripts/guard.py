import sys
import os.path
import re

def get_guard(file):
    relpath = os.path.relpath(file)
    if not relpath.endswith('.h'):
        sys.exit(f"invalid file type {relpath}")
    if 'include/' in relpath:
        is_global = True
        relpath = re.sub('^.*include/', '', relpath)
        prefix = '__'
    else:
        is_global = False
        prefix = '_'
    guard = prefix + relpath.upper().replace('/', '_').replace('.', '_')
    for c in guard:
        if not (c.isupper() or c == '_'):
            raise Exception(f"cannot find valud guard for {file}")
    return is_global, guard
