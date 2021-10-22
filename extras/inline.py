#!/usr/bin/env python3

import re
import sys
from pathlib import Path

SLI_LIBPATH = Path("lib/sli")
def replaced(filename, noinclude=None):
    if noinclude is None:
        noinclude = []
    sliimport = re.compile(r'\((?P<name>[^)]+)\) run')
    for no, line in enumerate(filename.open('r', encoding="utf8")):
        found = False
        for match in sliimport.finditer(line):
            incfile = (SLI_LIBPATH / match.group('name')).with_suffix(".sli")
            if incfile in noinclude:
                continue
            if not incfile.is_file():
                yield(f"%%% INSERT FAILED FOR >>> {incfile} <<<\n")
                yield(f"%%% {line}")
                continue
            yield(f"%%% INSERT {incfile} LITERALLY\n")
            yield(f"%%% L{no}: {line}")
            noinclude.append(incfile)
            for includedline in replaced(incfile, noinclude):
                yield includedline.replace('"', "'")
            yield(f"%%% END {incfile}\n")
            found = True
        if not found:
            yield f"{line}"

def main():
    filename = Path(sys.argv[1])

    for line in replaced(filename):
        print(line, end='')


if __name__ == '__main__':
    main()
