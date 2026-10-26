#!/usr/bin/env python3
"""Compile external Doxygen Markdown blocks into a native clangd YAML index.

No source edits, generated declarations, editor middleware or third-party Python
packages. Build dev BMIs first. Run verify.py after generation and restart clangd.
"""
import argparse
import json
from pathlib import Path
import re
import sys
from urllib.parse import unquote, urlparse

from lsp import Client

ROOT = Path(__file__).resolve().parents[2]
CATALOG = ROOT / "docs/functions/catalog.json"
OUTPUT = ROOT / "build/docs"


def tokens(text):
    return re.findall(r"[A-Za-z_]\w*|::|&&|[^\s]", text)


def load_entries():
    entries = json.loads(CATALOG.read_text())
    identifiers = set()
    for entry in entries:
        if entry["anchor"] in identifiers:
            raise ValueError(f"duplicate anchor: {entry['anchor']}")
        identifiers.add(entry["anchor"])
        document = ROOT / entry["document"]
        text = document.read_text()
        marker = f'<a id="{entry["anchor"]}"></a>'
        if text.count(marker) != 1:
            raise ValueError(f"{document}: anchor must occur once: {marker}")
        section = text.split(marker, 1)[1].split('<a id="', 1)[0]
        blocks = re.findall(r"```doxygen\n(.*?)\n```", section, re.S)
        if len(blocks) != 1:
            raise ValueError(f"{document}: expected one Doxygen block for {entry['anchor']}")
        block = blocks[0]
        fn = re.search(r"^@fn (.+)$", block, re.M)
        brief = re.search(r"^@brief (.+)$", block, re.M)
        if not fn or not brief:
            raise ValueError(f"{entry['anchor']}: @fn and @brief are required")
        full_name = entry["qualified"]
        expected = entry.get("fn", entry["signature"].replace(entry["spelling"], full_name, 1))
        if tokens(fn[1]) != tokens(expected):
            raise ValueError(f"{entry['anchor']}: @fn disagrees with catalog signature")
        entry = dict(entry, brief=brief[1],
                     documentation=re.sub(r"^@fn .+\n?", "", block, flags=re.M))
        yield entry


def locate(entry, text):
    signature = entry["signature"]
    pattern = r"\s*".join(re.escape(token) for token in tokens(signature))
    matches = list(re.finditer(pattern, text))
    if len(matches) != 1:
        raise ValueError(f"{entry['source']}: signature changed or ambiguous: {signature}")
    match = matches[0]
    # Match the declared identifier, not an occurrence in an argument or body.
    name = entry["spelling"].split("::")[-1]
    prefix = text[match.start():match.end()].split("(", 1)[0]
    return match.start() + prefix.rindex(name)


def symbol_location(location):
    path = Path(unquote(urlparse(location["uri"]).path))
    lines = path.read_text().splitlines()
    result = {"FileURI": location["uri"]}
    for name in ("start", "end"):
        position = location["range"][name]
        line = lines[position["line"]]
        # LSP uses UTF-16 units; clangd index locations use UTF-8 byte columns.
        prefix = line.encode("utf-16-le")[:position["character"] * 2].decode("utf-16-le")
        result[name.title()] = {"Line": position["line"], "Column": len(prefix.encode())}
    return result


def yaml_symbol(entry, info):
    rows = ["--- !Symbol"]
    fields = {"ID": info["id"], "Name": info["name"], "Scope": info["containerName"] or ""}
    for key, value in fields.items():
        rows.append(f"{key}: {json.dumps(value, ensure_ascii=False)}")
    rows += ["SymInfo:", "  Kind: " + entry["kind"], "  Lang: Cpp"]
    for name, field in (("declarationRange", "CanonicalDeclaration"),
                        ("definitionRange", "Definition")):
        if name not in info:
            continue
        location = symbol_location(info[name])
        rows += [field + ":", "  FileURI: " + json.dumps(location["FileURI"])]
        for point in ("Start", "End"):
            rows += ["  " + point + ":"]
            rows += [f"    {key}: {value}" for key, value in location[point].items()]
    rows += ["Documentation: |"]
    rows += ["  " + line for line in entry["documentation"].splitlines()]
    rows.append("...")
    return "\n".join(rows)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--clangd", required=True)
    args = parser.parse_args()
    entries = list(load_entries())
    OUTPUT.mkdir(parents=True, exist_ok=True)
    files, records, symbols = {}, [], []
    ids = set()
    inventory_entries = [entry for entry in entries if entry.get('usr')]
    resolved = {}
    if inventory_entries:
        from inventory import fingerprint
        inventory_path = OUTPUT/'kernel-inventory.json'
        inventory = json.loads(inventory_path.read_text())
        if inventory.get('profile') != fingerprint(args.clangd):
            raise ValueError('kernel inventory is stale; run tools/clangd_docs/inventory.py first')
        for item in inventory['functions'] + inventory.get('extra_functions', []):
            resolved.setdefault((item['source'], item['info']['usr']), []).append(item)
    with Client(args.clangd, ROOT) as client:
        for entry in entries:
            source = entry['source']
            if entry.get('indexable', True) is False:
                records.append({'anchor': entry['anchor'], 'qualified': entry['qualified'],
                                'source': source, 'document': entry['document'],
                                'brief': entry['brief'], 'indexable': False,
                                'reason': entry['reason']})
                continue
            if entry.get('usr'):
                matches = resolved.get((source, entry['usr']), [])
                if entry.get('source_sha256'):
                    matches = [item for item in matches if item['sha256'] == entry['source_sha256']]
                if not matches:
                    raise ValueError(f"{entry['anchor']}: semantic binding/body changed; review its documentation")
                info = matches[0]['info']
            else:
                if source not in files:
                    files[source] = client.open(source)
                query = client.at(source, files[source], locate(entry, files[source]))
                found = client.request('textDocument/symbolInfo', query)
                if not found or len(found) != 1:
                    raise ValueError(f"{entry['anchor']}: expected one resolved symbol: {found}")
                info = found[0]
            if (info.get('containerName') or '') + info['name'] != entry['qualified']:
                raise ValueError(f"{entry['anchor']}: wrong symbol: {info}")
            if info['id'] in ids:
                raise ValueError(f"duplicate symbol ID: {info['id']}")
            ids.add(info['id'])
            symbols.append(yaml_symbol(entry, info))
            records.append({'anchor': entry['anchor'], 'qualified': entry['qualified'],
                            'id': info['id'], 'usr': info['usr'],
                            'source': source, 'document': entry['document'],
                            'brief': entry['brief'], 'indexable': True,
                            'locations': [info[key]['uri'] for key in
                                          ('declarationRange', 'definitionRange') if key in info]})
    # Atomically replace complete artifacts; never leave a half-written index.
    for name, value in (("functions.idx", "\n".join(symbols) + "\n"),
                        ("symbols.json", json.dumps(records, ensure_ascii=False, indent=2) + "\n")):
        temporary = OUTPUT / (name + ".tmp")
        temporary.write_text(value)
        temporary.replace(OUTPUT / name)
    print(f"Generated {len(entries)} Doxygen symbol documents: {OUTPUT / 'functions.idx'}")


if __name__ == "__main__":
    try:
        main()
    except (ValueError, RuntimeError, OSError) as error:
        print(error, file=sys.stderr)
        sys.exit(1)
