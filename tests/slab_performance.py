#!/usr/bin/env python3
"""Persist timings from the real native slab/page allocator implementations."""
import argparse
import json
from pathlib import Path
import platform
import subprocess
import sys

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--binary', required=True)
p.add_argument('--allocator', required=True, choices=['buddy', 'freelist'])
p.add_argument('--output', required=True, type=Path)
a = p.parse_args()
run = subprocess.run([a.binary, a.allocator, '--benchmark'], capture_output=True, text=True)
print(run.stdout, end='', flush=True)
print(run.stderr, end='', file=sys.stderr, flush=True)
rows = [json.loads(line) for line in run.stdout.splitlines() if line.startswith('{')]
result = dict(physical_allocator=a.allocator, host=platform.platform(),
              executable=a.binary, exit_code=run.returncode, samples=rows)
a.output.parent.mkdir(parents=True, exist_ok=True)
a.output.write_text(json.dumps(result, indent=2) + '\n')
if run.returncode:
    sys.exit(1)
expected = {(kind, size, retain) for kind in ['freelist-slab', 'bitmap-slab']
            for size in [64, 256, 1800] for retain in [0, 1]}
assert len(rows) == 12 and {(r['implementation'], r['object_bytes'], r['retained_empty_limit']) for r in rows} == expected
assert all(r['ratio'] < .9 and r['slab_page_allocations'] == 1 and
           r['direct_page_allocations'] == 700000 and r['rounds'] == 7 for r in rows)
