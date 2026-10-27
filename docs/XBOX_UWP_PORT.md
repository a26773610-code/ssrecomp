# Xbox UWP port (experimental)

This branch is an initial staging area for a possible Xbox UWP x64 port of ssrecomp.

## Status

- [x] Separate development branch
- [x] UWP local storage path helper (not yet wired into the build)
- [ ] Audit SDL2 UWP support and melonDS dependencies
- [ ] Create UWP application entry point and package manifest
- [ ] Integrate UWP-specific save/configuration paths
- [ ] Add Xbox gamepad support
- [ ] Add Windows-hosted CI build and verify UWP SDK/toolchain
- [ ] Test installation and runtime in Xbox Developer Mode

No functional Xbox build is available yet. The helper in frontend/uwp is not yet compiled or called. Do not commit copyrighted game ROMs, BIOS files or private signing keys.

## Distribution

A build that runs in Developer Mode does not imply that it can be installed in Retail Mode. Retail distribution requires an authorized Microsoft-supported route and is not guaranteed for new UWP games.
