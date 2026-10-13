"""Cross-language deterministic sanity check, with sibling python-CLP.

Not a complete rules equivalence test. Verifies state and legal move parity for a
short sequence. Run with: python tests/test_cross_language.py build/clp_core_check ../python-CLP
"""
import json
import subprocess
import sys
from pathlib import Path


def query(exe, request):
    r = subprocess.run([str(exe)], input=json.dumps(request) + '\n', text=True,
                       capture_output=True, timeout=30, check=True)
    response = json.loads(r.stdout)
    if isinstance(response, dict) and 'error' in response:
        raise AssertionError(response)
    return response


def py_signature(m):
    return (m.kind, m.src, m.dst, m.ptype.name, m.facing, m.rotate_after)


def cpp_signature(m):
    coord = lambda v: tuple(v) if v is not None else None
    return (m['kind'], coord(m['src']), coord(m['dst']), m['ptype'], m['facing'], m['rotate_after'])


def check_state(game, state):
    from engine.pieces import Player
    b = state['position']['board']
    for i, cell in enumerate(b):
        stack = game.board.sq(i // 9, i % 9).stack
        expected = [(q.ptype.name, q.owner.name, q.facing) for q in stack]
        actual = [(q['type'], q['owner'], q['facing']) for q in cell]
        assert expected == actual, (i, expected, actual)
    assert game.current.name == state['position']['to_move']
    for hand in ('reserve', 'prisoners'):
        for side in Player:
            py_values = sorted(x.name for x in getattr(game.board, hand)[side])
            cpp_values = sorted(state['position'][hand][side.name])
            assert py_values == cpp_values, (hand, side, py_values, cpp_values)


def main():
    exe = Path(sys.argv[1]).resolve()
    sys.path.insert(0, str(Path(sys.argv[2]).resolve()))
    from engine.game import Game
    from engine import rules
    game = Game()
    state = query(exe, {'command': 'initial'})
    for ply in range(8):
        check_state(game, state)
        py_moves = rules.legal_moves(game.board, game.current)
        cpp_moves = query(exe, {'command': 'legal', 'state': state})
        pset = {py_signature(m) for m in py_moves}
        cset = {cpp_signature(m) for m in cpp_moves}
        assert pset == cset, (ply, len(pset), len(cset), pset - cset, cset - pset)
        sig = sorted(pset, key=str)[(ply * 7) % len(pset)]
        py_move = next(m for m in py_moves if py_signature(m) == sig)
        cpp_move = next(m for m in cpp_moves if cpp_signature(m) == sig)
        assert game.make_move(py_move)
        state = query(exe, {'command': 'apply', 'state': state, 'move': cpp_move})
    check_state(game, state)
    print('PASS: Python ↔ C++ matched 8 successive legal sets and resulting board/hands')


if __name__ == '__main__':
    main()
