#!/usr/bin/env python3
"""Compile valid cycles and reject malformed structures and callback contracts."""
import argparse
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
PREFIX = '''#include "kernel/support/flow.h"
using namespace xv::flow;
struct A; struct B; struct Missing;
struct Ctx {};
struct Pred { static bool test(Ctx&) noexcept { return true; } };
'''
CASES = [
    ('cycle', 'graph<A, types<node<A, noop, go<A>>>>', None),
    ('duplicate', 'graph<A, types<node<A, noop, stop>, node<A, noop, stop>>>', 'duplicate node id'),
    ('entry', 'graph<Missing, types<node<A, noop, stop>>>', 'invalid entry'),
    ('empty', 'graph<A, types<>>', 'empty graph'),
    ('jump', 'graph<A, types<node<A, noop, go<Missing>>>>', 'target does not exist'),
    ('false_branch', 'graph<A, types<node<A, noop, branch<Pred, stop, go<Missing>>>>>', 'target does not exist'),
    ('call_entry', 'graph<A, types<node<A, noop, call<Missing, A>>>>', 'target does not exist'),
    ('call_resume', 'graph<A, types<node<A, noop, call<A, Missing>>>>', 'target does not exist'),
    ('unreachable', 'graph<A, types<node<A, noop, stop>, node<B, noop, go<Missing>>>>', 'target does not exist'),
]
CALLBACKS = [
    ('action_result', 'static int run(Ctx&) noexcept { return 1; }', 'stop', 'action must return void'),
    ('action_throws', 'static void run(Ctx&) {}', 'stop', 'action must be noexcept'),
    ('predicate_result', 'static void run(Ctx&) noexcept {}', 'branch<Bad, stop, stop>', 'predicate must return bool'),
    ('predicate_throws', 'static void run(Ctx&) noexcept {}', 'branch<Bad, stop, stop>', 'predicate must be noexcept'),
]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--cxx', required=True)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='flow-compile-') as directory:
        source = Path(directory) / 'check.cpp'

        def check(name, code, expected):
            source.write_text(PREFIX + code)
            result = subprocess.run(
                [args.cxx, '-std=c++20', '-ffreestanding', '-nostdinc++',
                 '-fno-exceptions', '-fno-rtti', '-Wall', '-Wextra', '-Werror',
                 '-I', str(ROOT), '-fsyntax-only', str(source)],
                capture_output=True, text=True, timeout=30)
            assert (result.returncode == 0) if expected is None else (
                result.returncode != 0 and expected in result.stderr), (name, result.stderr)

        for name, graph, expected in CASES:
            check(name, f'using G = {graph};\nstatic_assert(sizeof(G) > 0);\n', expected)
        for name, action, transition, expected in CALLBACKS:
            predicate = ('static int test(Ctx&) noexcept { return 1; }'
                         if name == 'predicate_result' else
                         'static bool test(Ctx&) { return true; }')
            check(name, f'''struct Action {{ {action} }};
struct Bad {{ {predicate} }};
using G = graph<A, types<node<A, Action, {transition}>>>;
outcome invoke(Ctx& ctx) {{ return G::run(ctx); }}
''', expected)
        check('adapted_action_result', """struct BadAction { static int run(Ctx&) noexcept { return 1; } };
using G = graph<A, types<node<A, action<&BadAction::run>, stop>>>;
outcome invoke(Ctx& ctx) { return G::run(ctx); }
""", 'action must return void')
        check('adapted_condition_result', """struct BadCondition { static int test(Ctx&) noexcept { return 1; } };
using G = graph<A, types<node<A, noop, branch<condition<&BadCondition::test>, stop, stop>>>>;
outcome invoke(Ctx& ctx) { return G::run(ctx); }
""", 'predicate must return bool')
    print(f'flow compile checks: {len(CASES) + len(CALLBACKS) + 2} passed')


if __name__ == '__main__':
    main()
