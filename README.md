# cpp-CLP — Cờ Long Phượng C++ Rules Core

**Status: extraction candidate; NOT a frozen v1.0.0 release.**

This repository contains the C++ rules implementation extracted **unchanged** from the user's Ryuugin source snapshot, not the Ryuugin search engine. `core/core.h` exposes position, legal moves, transitions, checks, repetition and record helpers. The historical Python rules were created first and are preserved separately in `python-CLP`.

## Requirements

CMake 3.20+, C++20 compiler (MSVC / GCC / Clang); Python 3 for CTest smoke.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Optional Windows JSON DLL:

```powershell
cmake -S . -B build-msvc -DCLP_BUILD_RULES_SERVICE=ON
cmake --build build-msvc --config Release
```

C++ consumers can link to `clp::core` using `add_subdirectory` or an equivalent pinned dependency. The historical namespace is `ryuugin`; renaming it requires a separate, tested API migration and is intentionally **not** part of extraction.

- Source files: `core/core.cpp`, `core/core.h`, `core/core_search.h`, `payload.h`.
- Diagnostics: `core/core_check.cpp` (separate executable).
- Optional Windows service: `service/rules_service.cpp`.
- **Not included**: Ryuugin search (`search.cpp`, `search.h`, `tactical.cpp`, `tactical.h`, `classic.cpp`) or `protocol_smoke.cpp`; these remain Ryuugin tooling, not the rules library.

## Release gate

The smoke suite validates only basic behavior. Before announcing `v1.0.0`, compare rule parity with `python-CLP` over comprehensive fixtures (facing, full-stack captures, team moves, reserve/prisoners, repetition3, max plies, notation, checkmate), perform Windows MSVC build and agree on license. See `RELEASE_CHECKLIST.md`.

## Optional cross-language parity smoke

With sibling `python-CLP` extracted alongside this repository:

```bash
python tests/test_cross_language.py build/clp_core_check ../python-CLP
```

The script compares legal-move sets, board, stacks and hands over eight deterministic plies. This is an initial sanity test, not an exhaustive proof.
