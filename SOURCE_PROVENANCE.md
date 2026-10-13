# Source provenance

Source snapshots supplied by the user in conversation 2026-10-13; no source files were edited by extraction.

- `cpp-CLP`: five files from `src.rar` + separately provided `payload.h`. Ryuugin Search excluded.
- `python-CLP`: eight Python source files from `engine(1).rar`; generated `__pycache__` excluded.
- Build configuration, smoke tests, README and manifest are new packaging files, not upstream core modifications.

| Relative path | SHA-256 (source bytes) |
|---|---|
| `core/core.cpp` | `d8369d7ac300bc3b8f7d6dd4aaa91ce171d115e7e8127f3776245e5ed4af7270` |
| `core/core.h` | `ac32a97082a3d5410c21de649b76cc6a8a294c4992db6675473423b20ec90c14` |
| `core/core_search.h` | `ba9f2960434869e8ed569fd75da48c34a85af0ec50a1e055dc5105960198f9bd` |
| `core/core_check.cpp` | `1a7173789005be202793748c76216a375a8e5d9c88a07b906e8fb5b9eb647c02` |
| `service/rules_service.cpp` | `bee9b318154ed954ac60d956d59023307262e513e0387e62fed31bc877b1f13d` |
| `payload.h` | `3a992398e734d82bab9a2216ed825d1642d22587ca7d4e50e6a8a647c457dad4` |

These checksums document the supplied archives, not an audited origin commit in the upstream Ryuugin repository.
