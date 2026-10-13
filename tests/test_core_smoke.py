"""Basic contract smoke for the unmodified Ryuugin CLP Core diagnostic CLI."""
import json
import subprocess
import sys


def run(cli, request):
    proc = subprocess.run([cli], input=json.dumps(request) + "\n", text=True,
                          stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=15, check=True)
    result = json.loads(proc.stdout.strip())
    assert "error" not in result, result
    return result


def main():
    exe = sys.argv[1]
    state = run(exe, {"command": "initial"})
    assert state["schema"] == "clp.state/1"
    assert len(state["position"]["board"]) == 81
    legal = run(exe, {"command": "legal", "state": state})
    assert len(legal) == 18, len(legal)
    roundtrip = run(exe, {"command": "roundtrip", "state": state})
    assert roundtrip == state
    move = legal[0]
    advanced = run(exe, {"command": "apply", "state": state, "move": move})
    assert advanced["ply"] == state["ply"] + 1
    undone = run(exe, {"command": "undo", "state": state, "move": move})
    assert undone == state
    print("PASS: C++ initial, 18 legal moves, roundtrip, apply and undo")


if __name__ == "__main__":
    main()
