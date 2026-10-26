#!/usr/bin/env python3
"""Verify native Doxygen hover, symbol binding and navigation using real clangd.

Known source-comment precedence is reported explicitly, never counted as an
external hover success. No editor proxy or generated source buffers are used.
"""
import argparse
import hashlib
import json
from pathlib import Path
import sys
import time

from build import ROOT, OUTPUT, load_entries, locate
from lsp import Client


def hover_text(result):
    if not result:
        return ""
    contents = result.get("contents", "")
    if isinstance(contents, dict):
        return contents.get("value", "")
    if isinstance(contents, list):
        return "\n".join(item if isinstance(item, str) else item.get("value", "")
                         for item in contents)
    return contents


def hover_when_loaded(client, query, brief, retry=True):
    deadline = time.monotonic() + (5 if retry else 0)
    while True:
        result = hover_text(client.request("textDocument/hover", query))
        if brief in result or time.monotonic() >= deadline:
            return result
        time.sleep(0.05)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--clangd", required=True)
    parser.add_argument("--background-index", action="store_true",
                        help="also verify with the editor's normal background indexing enabled")
    args = parser.parse_args()
    entries = list(load_entries())
    generated = {item["anchor"]: item for item in json.loads((OUTPUT / "symbols.json").read_text())}
    files = {entry["source"] for entry in entries}
    inventory = json.loads((OUTPUT / 'kernel-inventory.json').read_text())
    positions = {}
    for item in inventory['functions']:
        positions.setdefault((item['source'], item['info']['usr']), []).append(item)
    before = {name: hashlib.sha256((ROOT / name).read_bytes()).hexdigest() for name in files}
    report, buffers, samples = [], {}, []
    with Client(args.clangd, ROOT, config=True, background=args.background_index) as client:
        for index, entry in enumerate(entries):
            source = entry["source"]
            if not entry.get('indexable', True):
                report.append({'symbol': entry['qualified'], 'outcome': 'documentation-only',
                               'reason': entry['reason'], 'document': entry['document'],
                               'anchor': entry['anchor']})
                continue
            if source not in buffers:
                buffers[source] = client.open(source)
            text = buffers[source]
            if entry.get('usr'):
                candidates = positions[(source, entry['usr'])]
                item = next((value for value in candidates
                             if value['sha256'] == entry.get('source_sha256')), candidates[0])
                query = {'textDocument': {'uri': (ROOT/source).as_uri()},
                         'position': item['selection']['start']}
            else:
                query = client.at(source, text, locate(entry, text))
            info = client.request("textDocument/symbolInfo", query)
            assert info and info[0]["id"] == generated[entry["anchor"]]["id"], info
            external = entry["hover_expectation"] == "external"
            rendered = hover_when_loaded(client, query, entry["brief"], retry=external)
            has_external = entry["brief"] in rendered
            if external:
                assert has_external, f"{entry['qualified']}: external documentation missing: {rendered}"
                assert "@brief" not in rendered and "### Brief" in rendered, rendered
                outcome = "external-doxygen"
            elif entry['hover_expectation'] == 'source-comment':
                assert rendered and not has_external, f"source-comment expectation changed: {entry['qualified']}"
                outcome = "source-comment-precedence"
            else:
                if has_external:
                    assert '@brief' not in rendered and '### Brief' in rendered, rendered
                    outcome = 'external-doxygen'
                else:
                    outcome = 'native-documentation-priority-or-index-limit'
                if not rendered:
                    outcome = 'no-native-hover' 
            definitions = client.request("textDocument/definition", query)
            if entry['hover_expectation'] != 'observe':
                assert definitions, f"definition lost: {entry['qualified']}"
            targets = definitions if isinstance(definitions, list) else [definitions]
            allowed = generated[entry["anchor"]]["locations"]
            if definitions:
                assert any(target.get("targetUri", target.get("uri", "")) in allowed
                           for target in targets), (entry['qualified'], targets, allowed)
            report.append({"symbol": entry["qualified"], "outcome": outcome,
                           "document": entry["document"], "anchor": entry["anchor"]})
            samples.append(f"## {entry['qualified']}\n\nOutcome: `{outcome}`\n\n{rendered}\n")
            if index % 100 == 0:
                print(f"Verified {index+1}/{len(entries)} entries", flush=True)
            # Bound retained ASTs during a whole-kernel scan.
            if len(buffers) > 8:
                oldest = next(iter(buffers))
                client.notify('textDocument/didClose', {'textDocument': {'uri': (ROOT/oldest).as_uri()}})
                del buffers[oldest]
        # Same symbol at its declaration and in different module consumers.
        checks = [
            ("kernel/process/wait/wait_channels.cppm", "void prepare(", "prepare(",
             "xv::process::WaitChannels::prepare"),
            ("kernel/process/wait/sleep.cpp", "wait_channels().prepare(", "prepare(",
             "xv::process::WaitChannels::prepare"),
            ("kernel/process/lifecycle/process_manager.cpp", "waits_.wake(channel)", "wake(",
             "xv::process::WaitChannels::wake"),
            ("kernel/syscall/sysproc.cpp", "xv::trap::clock().pause(count)", "pause(",
             "xv::trap::TickClock::pause"),
            ("kernel/fs/platform/kernel_platform.cpp", "xv::process::wakeup(channel)", "wakeup(",
             "xv::process::wakeup"),
        ]
        by_name = {entry["qualified"]: entry for entry in entries}
        for source, expression, token, name in checks:
            if source not in buffers:
                buffers[source] = client.open(source)
            text = buffers[source]
            start = text.index(expression)
            query = client.at(source, text, start + expression.index(token))
            entry = by_name[name]
            rendered = hover_when_loaded(client, query, entry["brief"])
            assert entry["brief"] in rendered, (source, name, rendered)
            report.append({"symbol": name, "outcome": "cross-module-doxygen", "source": source})
    after = {name: hashlib.sha256((ROOT / name).read_bytes()).hexdigest() for name in files}
    assert before == after, "source files changed during documentation verification"
    counts = {}
    for row in report:
        counts[row["outcome"]] = counts.get(row["outcome"], 0) + 1
    (OUTPUT / "hover-report.json").write_text(json.dumps(
        {"background_index": args.background_index, "counts": counts, "checks": report}, ensure_ascii=False, indent=2) + "\n")
    (OUTPUT / "hover-samples.md").write_text("# Actual native clangd hover responses\n\n" + "\n".join(samples))
    print(json.dumps(counts, ensure_ascii=False))
    print("Doxygen rendering, declaration/call-site hover, definition and source integrity: passed")


if __name__ == "__main__":
    try:
        main()
    except (AssertionError, RuntimeError, ValueError, OSError) as error:
        print(error, file=sys.stderr)
        sys.exit(1)
