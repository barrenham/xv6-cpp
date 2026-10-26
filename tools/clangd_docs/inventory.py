#!/usr/bin/env python3
"""Inventory kernel C++ functions using clangd AST identities, without source edits."""
import argparse
import hashlib
import json
from pathlib import Path
import re
from lsp import Client

ROOT = Path(__file__).resolve().parents[2]
FUNCTION_KINDS = {6, 9, 12}


def offset(text, position):
    lines = text.splitlines(keepends=True)
    line = lines[position['line']]
    prefix = line.encode('utf-16-le')[:position['character'] * 2].decode('utf-16-le')
    return sum(map(len, lines[:position['line']])) + len(prefix)


def functions(symbols, parents=()):
    for symbol in symbols or []:
        if 'selectionRange' not in symbol:
            continue
        if symbol['kind'] in FUNCTION_KINDS:
            yield symbol, parents
        yield from functions(symbol.get('children'), parents + (symbol['name'],))


def scan(client, source, *, strict=True):
    text = client.open(source, strict=strict)
    symbols = client.request('textDocument/documentSymbol', {
        'textDocument': {'uri': (ROOT / source).as_uri()}})
    records = []
    for symbol, parents in functions(symbols):
        start, end = (offset(text, symbol['range'][key]) for key in ('start', 'end'))
        selected = offset(text, symbol['selectionRange']['start'])
        raw = text[start:end]
        query = {'textDocument': {'uri': (ROOT / source).as_uri()},
                 'position': symbol['selectionRange']['start']}
        infos = client.request('textDocument/symbolInfo', query) or []
        info = infos[0] if len(infos) == 1 else None
        record = {'source': source, 'name': symbol['name'], 'parents': list(parents),
                  'kind': symbol['kind'], 'detail': symbol.get('detail', ''),
                  'range': symbol['range'], 'selection': symbol['selectionRange'],
                  'raw': raw, 'selection_offset': selected-start,
                  'sha256': hashlib.sha256(raw.encode()).hexdigest(), 'info': info}
        records.append(record)
    client.notify('textDocument/didClose', {'textDocument': {'uri': (ROOT / source).as_uri()}})
    return records


def scan_additional(clangd):
    """Discover inactive lab declarations and shared include bodies, docs only.

    This is a parsing profile, not a successful alternate kernel build. No IDs
    from this profile are installed into the default native documentation index.
    """
    database = ROOT / 'build/docs/conditional-cdb'
    database.mkdir(exist_ok=True)
    commands = json.loads((ROOT / 'build/dev/compile_commands.json').read_text())
    for command in commands:
        command['command'] += ' -DLAB_LOCK -DLAB_NET'
    include = ROOT / 'kernel/arch/riscv/register_operations.inc'
    commands.append({
        'directory': str(ROOT), 'file': str(include),
        'arguments': [str(Path(clangd).parent / 'clang++'),
                      '--target=riscv64-unknown-elf', '-std=c++20', '-I' + str(ROOT),
                      '-include', 'kernel/arch/riscv/register_constants.h',
                      '-x', 'c++', '-c', str(include)],
    })
    (database / 'compile_commands.json').write_text(json.dumps(commands))
    extra = []
    with Client(clangd, ROOT, compilation_database=database) as client:
        for source in ['kernel/support/stats.cpp', 'kernel/syscall/handlers.cppm',
                       'kernel/syscall/sysproc.cpp', str(include.relative_to(ROOT))]:
            records = scan(client, source, strict=False)
            for item in records:
                item['configuration'] = 'include-context' if source.endswith('.inc') else 'LAB_LOCK/LAB_NET'
                item['configuration_diagnostics'] = client.last_diagnostics
            extra.extend(records)
    return extra


def fingerprint(clangd):
    import subprocess
    files = {str(path.relative_to(ROOT)): hashlib.sha256(path.read_bytes()).hexdigest()
             for path in (ROOT/'kernel').rglob('*') if path.is_file() and
             path.suffix in {'.cpp', '.cppm', '.h', '.hpp', '.inc', '.S'}}
    return {'files': files,
            'compile_commands': hashlib.sha256((ROOT/'build/dev/compile_commands.json').read_bytes()).hexdigest(),
            'clangd': subprocess.check_output([clangd, '--version'], text=True).splitlines()[0]}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--clangd', required=True)
    args = parser.parse_args()
    sources = sorted(str(path.relative_to(ROOT)) for path in (ROOT/'kernel').rglob('*')
                     if path.suffix in {'.cpp', '.cppm', '.h', '.hpp'})
    result = {'profile': fingerprint(args.clangd), 'files': [], 'functions': [], 'errors': []}
    with Client(args.clangd, ROOT) as client:
        for index, source in enumerate(sources):
            try:
                found = scan(client, source, strict=False)
                result['files'].append({'source': source, 'functions': len(found),
                                        'diagnostics': client.last_diagnostics})
                result['functions'].extend(found)
            except Exception as error:
                result['errors'].append({'source': source, 'error': str(error)})
            if index % 15 == 0:
                print(f'{index+1}/{len(sources)} files, {len(result["functions"])} declarations', flush=True)
    result['extra_functions'] = scan_additional(args.clangd)
    destination = ROOT/'build/docs/kernel-inventory.json'
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_text(json.dumps(result, ensure_ascii=False, indent=2)+'\n')
    print(f'{len(result["functions"])} declarations, {len(result["errors"])} errors: {destination}')


if __name__ == '__main__':
    main()
