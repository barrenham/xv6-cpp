#!/usr/bin/env python3
"""Append a versioned immutable asset directory to an ordinary boot ELF."""
import argparse
import os
from pathlib import Path
import struct
import tempfile


def unpack(data):
    if len(data) < 64 + 32 or data[:6] != b'\x7fELF\x02\x01':
        raise ValueError('boot bundle must contain a little-endian ELF64 executable')
    magic, version, entry_size, table, count, reserved = struct.unpack_from('<8sIIQII', data, len(data) - 32)
    if (magic != b'XVBOOT01' or version != 1 or entry_size != 88 or reserved or
            not 0 < count <= 128 or table + count * 88 != len(data) - 32):
        raise ValueError('invalid boot asset directory')
    assets, names, first = [], set(), table
    for index in range(count):
        raw_name, offset, size, flags, reserved = struct.unpack_from('<64sQQII', data, table + index * 88)
        name, separator, _ = raw_name.partition(b'\0')
        if (not separator or not name or any(c < 33 or c > 126 for c in name) or
                name in names or not size or offset > table or size > table - offset or
                flags & ~1 or reserved):
            raise ValueError('invalid boot asset entry')
        names.add(name)
        first = min(first, offset)
        assets.append((name.decode('ascii'), data[offset:offset + size], bool(flags)))
    # Assets must follow the complete ELF, including its optional section table.
    phoff, shoff = struct.unpack_from('<QQ', data, 32)
    ehsize, phsize, phcount, shsize, shcount = struct.unpack_from('<5H', data, 52)
    if (ehsize != 64 or phsize != 56 or not phcount or phcount > 64 or
            phoff + phsize * phcount > first or
            (shcount and (shsize != 64 or shoff + shsize * shcount > first))):
        raise ValueError('assets overlap ELF metadata')
    for index in range(phcount):
        offset = struct.unpack_from('<Q', data, phoff + index * phsize + 8)[0]
        size = struct.unpack_from('<Q', data, phoff + index * phsize + 32)[0]
        if offset > first or size > first - offset:
            raise ValueError('assets overlap an ELF program segment')
    for index in range(shcount):
        kind = struct.unpack_from('<I', data, shoff + index * shsize + 4)[0]
        offset, size = struct.unpack_from('<QQ', data, shoff + index * shsize + 24)
        if kind != 8 and (offset > first or size > first - offset):
            raise ValueError('assets overlap an ELF section')
    return data[:first], assets


def pack(base, assets):
    data = bytearray(base)
    if data[:4] != b'\x7fELF':
        raise ValueError('bootstrap must be an ELF executable')
    entries = []
    names = set()
    for name, content, executable in assets:
        encoded = name.encode('ascii')
        if (not encoded or len(encoded) >= 64 or name in names or
                any(c < 33 or c > 126 for c in encoded) or not content):
            raise ValueError(f'invalid or duplicate asset {name!r}')
        names.add(name)
        entries.append((encoded, len(data), len(content), int(executable)))
        data.extend(content)
    if not entries or len(entries) > 128:
        raise ValueError('boot bundle must contain 1..128 assets')
    table = len(data)
    for name, offset, size, flags in entries:
        data.extend(struct.pack('<64sQQII', name, offset, size, flags, 0))
    data.extend(struct.pack('<8sIIQII', b'XVBOOT01', 1, 88, table, len(entries), 0))
    return data


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument('--base', type=Path)
    source.add_argument('--extend', type=Path, help='preserve an existing bundle and add or replace named assets')
    parser.add_argument('--output', required=True, type=Path)
    parser.add_argument('--asset', action='append', default=[], metavar='NAME=FILE')
    parser.add_argument('--executable', action='append', default=[], metavar='NAME=ELF')
    args = parser.parse_args()
    assets = []
    for executable, values in ((False, args.asset), (True, args.executable)):
        for value in values:
            name, separator, path = value.partition('=')
            if not separator:
                parser.error('assets require NAME=FILE')
            content = Path(path).read_bytes()
            if executable and content[:4] != b'\x7fELF':
                parser.error(f'executable asset {name!r} is not ELF')
            assets.append((name, content, executable))
    try:
        if args.extend:
            base, previous = unpack(args.extend.read_bytes())
            merged = {name: (name, content, executable) for name, content, executable in previous}
            supplied = set()
            for asset in assets:
                if asset[0] in supplied:
                    raise ValueError('duplicate replacement asset: ' + asset[0])
                supplied.add(asset[0])
                merged[asset[0]] = asset
            assets = list(merged.values())
        else:
            base = args.base.read_bytes()
        content = pack(base, assets)
    except (UnicodeError, ValueError) as error:
        parser.error(str(error))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(dir=args.output.parent, delete=False) as output:
        temporary = Path(output.name)
        try:
            output.write(content)
            output.flush()
            os.fsync(output.fileno())
        except BaseException:
            temporary.unlink(missing_ok=True)
            raise
    try:
        os.replace(temporary, args.output)
    finally:
        temporary.unlink(missing_ok=True)


if __name__ == '__main__':
    main()
