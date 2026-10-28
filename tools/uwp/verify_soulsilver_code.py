#!/usr/bin/env python3
"""Verify a user's private SoulSilver US ROM against the public ssrecomp code layout.

No ROM, decompressed sections, or copyrighted game data is uploaded or written.
Run: python3 tools/uwp/verify_soulsilver_code.py /path/to/SoulSilver.nds
"""
import argparse
import hashlib
import json
import os
import struct
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
sys.path.insert(0, os.path.join(ROOT, 'tools'))
from ndsrom import read_rom  # noqa: E402


def read_itcm(main, base):
    """Extract the one ITCM autoload block using the ROM's own autoload table."""
    offset = len(main) - 0x24
    entries = [struct.unpack_from('<3I', main, offset + i * 12) for i in range(2)]
    data_offset = offset - sum(item[1] for item in entries)
    for address, size, unused_bss in entries:
        if address == base:
            return main[data_offset:data_offset + size]
        data_offset += size
    raise ValueError('ITCM autoload section not found')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('rom', help='locally stored, legally obtained SoulSilver US .nds file')
    parser.add_argument('--layout', default=os.path.join(ROOT, 'layouts', 'soulsilver-us.json'))
    args = parser.parse_args()

    with open(args.layout, encoding='utf-8') as stream:
        layout = json.load(stream)
    if not os.path.isfile(args.rom):
        parser.error('ROM not found')
    main_image, overlays = read_rom(args.rom)
    problems = []
    for entry in layout['sections']:
        source = entry['source']
        if source == 'main':
            data = main_image
        elif source == 'itcm':
            data = read_itcm(main_image, entry['base'])
        else:
            if source not in overlays:
                problems.append(f"{entry['name']}: missing overlay {source}")
                continue
            data = overlays[source][1]
        got_sha1 = hashlib.sha1(data).hexdigest()
        if got_sha1 != entry['sha1'] or len(data) != entry['size']:
            problems.append(f"{entry['name']}: code hash or size differs from layout")
    if problems:
        print(f'INCOMPATIBLE: {len(problems)} / {len(layout["sections"])} sections failed:')
        for problem in problems[:20]:
            print(' -', problem)
        return 1
    print(f'COMPATIBLE: all {len(layout["sections"])} SoulSilver US code sections match.')
    print('This checks recompilation inputs only, not graphical assets, savedata or Xbox compatibility.')
    return 0


if __name__ == '__main__':
    sys.exit(main())
