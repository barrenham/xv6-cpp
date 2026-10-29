#!/usr/bin/env python3
"""Check reviewed source scope, semantic graph bindings and preserved subtrees.

This detects stale evidence; semantic reviews and runtime tests remain separate.
Run tools/clangd_docs/inventory.py before this checker after editing kernel code.
"""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
SUFFIXES = {'.cpp', '.cppm', '.h', '.hpp', '.inc', '.S'}
AUDITS = [
    'docs/process-flow-audit.json', 'docs/memory-flow-audit.json',
    'docs/syscall-flow-audit.json', 'docs/kernel-primitives-flow-audit.json',
    'docs/drivers-flow-audit.json', 'docs/fs-flow-audit.json',
]


def read(path):
    return json.loads((ROOT / path).read_text())


def digest(path):
    return hashlib.sha256((ROOT / path).read_bytes()).hexdigest()


def reviewed_files(document):
    if 'files' in document:
        yield from document['files']
    for module in document.get('modules', []):
        yield from reviewed_files(module)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--inventory', default='build/docs/kernel-inventory.json')
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    manifest = read('docs/kernel-flow-source-manifest.json')
    actual = {str(p.relative_to(ROOT)) for p in (ROOT / 'kernel').rglob('*')
              if p.is_file() and p.suffix in SUFFIXES}
    indexed = {item['source']: item for item in manifest['files']}
    assert len(indexed) == len(manifest['files']), 'duplicate manifest source'
    assert actual == indexed.keys(), ('source set changed', actual ^ indexed.keys())
    for source, item in indexed.items():
        assert digest(source) == item['sha256'], ('source review is stale', source)
    reviews = {}
    for name in AUDITS:
        document = read(name)
        assert document['status'] == 'identified_complex_paths_connected', name
        for item in reviewed_files(document):
            source = item['source']
            assert source not in reviews, ('duplicate source review', source)
            assert not item.get('pending'), ('pending source work', source)
            assert item.get('review'), ('missing review', source)
            assert digest(source) == item['sha256'], ('stale review', source)
            reviews[source] = item
    expected = {p for p in actual if not p.startswith(('kernel/boot/', 'kernel/net/'))}
    assert expected == reviews.keys(), ('review scope mismatch', expected ^ reviews.keys())
    for prefix, before in manifest['preserved_subtrees'].items():
        after = {str(p.relative_to(ROOT)): digest(str(p.relative_to(ROOT)))
                 for p in (ROOT / prefix).rglob('*') if p.is_file()}
        assert before == after, ('preserved subtree changed', prefix)
    inventory = read(args.inventory)
    assert not inventory['errors'], inventory['errors']
    assert inventory['profile']['files'] == {p: digest(p) for p in actual}, 'inventory is stale'
    records = inventory['functions']
    extra = inventory.get('extra_functions', [])
    coverage = read('docs/kernel-flow-coverage.json')
    bindings = []
    graph_keys = []
    for entry in coverage['entries']:
        assert digest(entry['definition']) == entry['definition_sha256'], entry['definition']
        namespace = entry.get('namespace', 'xv::' + (
            'print' if entry['module'] == 'support' else entry['module']))
        prefix = (namespace + '::' if namespace else '') + (
            entry['owner'] + '::' if entry['owner'] else '')
        assert [b['qualified'] for b in entry['bindings']] == [prefix + m for m in entry['methods']]
        available = extra if entry.get('configuration') else records
        for binding in entry['bindings'] + entry.get('internal_bindings', []):
            matches = [r for r in available if r['info']['usr'] == binding['usr']
                       and r['source'] == binding['source']
                       and r['sha256'] == binding['source_sha256']]
            assert matches, ('stale semantic binding', binding['qualified'])
            used = {g for r in matches for g in re.findall(r'(\w+Flow::\w*Graph)::run', r['raw'])}
            for record in matches:
                local = re.findall(r'(?<![:\w])(\w*Graph)::run', record['raw'])
                if local:
                    owner = binding['qualified'].rsplit('::', 1)[0].rsplit('::', 1)[-1]
                    assert owner.endswith('Flow'), ('unresolved local graph scope', binding['qualified'])
                    used.update(owner + '::' + graph for graph in local)
            assert used == set(binding['graphs']), ('graph call changed', binding['qualified'])
            bindings.append(binding)
        assert set(entry['graphs']) == {
            g for b in entry['bindings'] + entry.get('internal_bindings', []) for g in b['graphs']}
        graph_keys.extend((entry['definition'], prefix, g) for g in entry['graphs'])
    assert len(graph_keys) == len(set(graph_keys)), 'duplicate graph registration'
    registered = {b['usr'] for b in bindings}
    unregistered = {(r['source'], (r['info'].get('containerName') or '') + r['info']['name'])
                    for r in records + extra if not r['source'].startswith('kernel/net/')
                    and re.search(r'\w+Flow::\w*Graph::run', r['raw'])
                    and r['info']['usr'] not in registered}
    assert not unregistered, ('unregistered graph runner', unregistered)
    network = read('docs/network-flow-coverage.json')
    assert len(network['graph_definitions']) == network['graph_definition_count']
    for graph in network['graph_definitions']:
        assert graph['source'] in actual, graph
    declared = Counter((str(p.relative_to(ROOT)), name)
                       for p in (ROOT / 'kernel/net').rglob('*')
                       if p.is_file() and p.suffix in SUFFIXES
                       for name in re.findall(r'\busing\s+(\w*Graph)\s*=\s*(?:xv::)?flow::graph\s*<', p.read_text()))
    assert declared == Counter((g['source'], g['graph']) for g in network['graph_definitions']), 'network graph definitions changed'
    non_network_declarations = sum(len(re.findall(r'\busing\s+\w*Graph\s*=\s*(?:xv::)?flow::graph\s*<',
                                                (ROOT / source).read_text()))
                                   for source in expected)
    alternatives = sum(len(v['conditions']) - 1 for e in coverage['entries']
                       for v in e.get('conditional_variants', []))
    assert non_network_declarations == len(graph_keys) + alternatives, 'unregistered graph definition or conditional variant'
    result = dict(
        source_files=len(actual), reviewed_non_network_non_boot=len(reviews),
        preserved_network_source_files=sum(p.startswith('kernel/net/') for p in actual),
        excluded_boot_source_files=sum(p.startswith('kernel/boot/') for p in actual),
        graphs=len(graph_keys), additional_conditional_graph_definitions=alternatives, original_entries=sum(len(e['bindings']) for e in coverage['entries']),
        internal_bindings=len(bindings) - sum(len(e['bindings']) for e in coverage['entries']),
        prior_network_graphs=network['graph_definition_count'],
        preserved_subtree_files={p: len(v) for p, v in manifest['preserved_subtrees'].items()},
        pending_sources=0, unregistered_runners=0,
        caveat='Source/binding integrity is evidence of reviewed coverage, not proof of runtime semantics or every build profile.')
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(result, ensure_ascii=False, indent=2) + '\n')
    print(json.dumps(result, ensure_ascii=False))


if __name__ == '__main__':
    main()
