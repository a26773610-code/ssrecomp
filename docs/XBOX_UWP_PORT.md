# Xbox UWP port (experimental)

This branch is an initial staging area for a possible Xbox UWP x64 port of ssrecomp.

## Status

- [x] Separate development branch
- [x] UWP local storage path helper wired into CMake and melonDS local-file calls
- [ ] Audit SDL2 UWP support and melonDS dependencies
- [ ] Create UWP application entry point and package manifest
- [x] Route melonDS local-file operations through UWP local path helper (initial implementation)
- [ ] Audit UTF-8/Unicode filename handling and all other save/configuration call sites
- [x] Add portable Xbox-to-DS gamepad button mapping and unit tests
- [ ] Read physical Xbox controller state via UWP Gaming.Input and connect to melonDS
- [x] Add Windows-hosted MSVC helper smoke test (desktop compilation only)
- [ ] Verify CI results and build with actual UWP SDK/toolchain
- [ ] Test installation and runtime in Xbox Developer Mode

No functional Xbox build is available yet. The helper is included in CMake and referenced by melonDS local-file functions, but UWP execution has not been compiled or tested. Do not commit copyrighted game ROMs, BIOS files or private signing keys.

## Distribution

A build that runs in Developer Mode does not imply that it can be installed in Retail Mode. Retail distribution requires an authorized Microsoft-supported route and is not guaranteed for new UWP games.
