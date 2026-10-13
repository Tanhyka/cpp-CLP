# Release gate (not yet complete)

- [x] Preserve Core C++ source snapshot without changing rule behavior
- [x] Provide CMake library + core checker and simple CTest smoke
- [ ] Comprehensive golden parity: historical Python vs C++ Ryuugin across representative CLP positions
- [ ] Exhaustive move/undo, stack 1–5, team/facing, hand/drop, repetition and terminal regression
- [ ] MSVC Windows CI, optional DLL/export smoke
- [ ] Define stable public API, semantic version scheme, explicit upstream provenance
- [ ] Choose and validate legal redistribution license before public visibility
- [ ] Freeze exact commit SHA and tag only when all gates pass

Do **not** treat a successful compile as full rules equivalence.
