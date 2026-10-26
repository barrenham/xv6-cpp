#!/usr/bin/env python3
"""Check complete kernel file/function documentation against a compiler inventory."""
import argparse
import json
from pathlib import Path
import re
from build import ROOT, OUTPUT, load_entries
from inventory import fingerprint


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--clangd', required=True)
    args = parser.parse_args()
    inventory = json.loads((OUTPUT/'kernel-inventory.json').read_text())
    assert inventory['profile'] == fingerprint(args.clangd), 'compiler inventory is stale'
    assert not inventory['errors'], inventory['errors']
    assert not [file for file in inventory['files'] if file['diagnostics']], 'default configuration has diagnostics'
    entries = list(load_entries())
    by_usr = {entry['usr']: entry for entry in entries if entry.get('usr')}
    assert len(by_usr) == sum('usr' in entry for entry in entries), 'duplicate semantic identities'
    functions = inventory['functions'] + inventory.get('extra_functions', [])
    expected = {item['info']['usr'] for item in functions}
    assert not expected - by_usr.keys(), ('undocumented functions', expected - by_usr.keys())
    assert not by_usr.keys() - expected, ('stale functions', by_usr.keys() - expected)
    for entry in entries:
        if not entry.get('usr'):
            assert entry['reason'] == 'assembly-entry', entry
            continue
        records = [item for item in functions if item['info']['usr'] == entry['usr'] and item['source'] == entry['source']]
        assert any(item['sha256'] == entry['source_sha256'] for item in records), ('body/signature needs documentation review', entry['qualified'])
    indexed_files = json.loads((ROOT/'docs/functions/files.json').read_text())
    assert {item['source'] for item in indexed_files} == set(inventory['profile']['files']), 'kernel file coverage mismatch'
    for item in indexed_files:
        document = ROOT/item['document']
        assert document.is_file(), document
        if item['source'].endswith('.S'):
            assert item.get('assembly_entries'), ('assembly entry missing', item)
    for path in (ROOT/'docs').rglob('*.md'):
        for link in re.findall(r'\]\(([^)]+)\)', path.read_text()):
            target, _, anchor = link.partition('#')
            if '://' in target or target.startswith('mailto:'):
                continue
            destination = path.parent/target if target else path
            assert destination.exists(), (str(path), link)
            if anchor and destination.suffix == '.md':
                assert f'id="{anchor}"' in destination.read_text(), (str(path), link)
    result = {'kernel_files': len(indexed_files), 'default_unique_symbols': len({item['info']['usr'] for item in inventory['functions']}),
              'all_cpp_symbols': len(expected), 'documentation_entries': len(entries),
              'indexable_entries': sum(entry.get('indexable', True) for entry in entries),
              'documentation_only': sum(not entry.get('indexable', True) for entry in entries),
              'missing_files': 0, 'missing_function_symbols': 0}
    (OUTPUT/'coverage.json').write_text(json.dumps(result, ensure_ascii=False, indent=2)+'\n')
    print(json.dumps(result, ensure_ascii=False))


if __name__ == '__main__':
    main()
