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
- [x] Implement Windows.Gaming.Input controller polling module (not yet runtime-tested)
- [ ] Connect polled buttons to melonDS frame input and validate on hardware
- [x] Add Windows-hosted MSVC helper smoke test (desktop compilation only)
- [ ] Verify CI results and build with actual UWP SDK/toolchain
- [ ] Test installation and runtime in Xbox Developer Mode

No functional Xbox build is available yet. The helper is included in CMake and referenced by melonDS local-file functions, but UWP execution has not been compiled or tested. Do not commit copyrighted game ROMs, BIOS files or private signing keys.

## Distribution

A build that runs in Developer Mode does not imply that it can be installed in Retail Mode. Retail distribution requires an authorized Microsoft-supported route and is not guaranteed for new UWP games.

## Build blocker: generated game sources

The repository does not include `gen/sources.cmake`, which the root CMakeLists.txt includes. The game cannot be built from the public source checkout alone. The existing generation workflow must be reviewed and run with a legally obtained ROM outside the public repository. Never upload ROMs or derived copyrighted game data to public GitHub Actions artifacts.

The gamepad reader currently polls the first UWP Gamepad and is not yet connected to the main frame loop. The Windows CI smoke test is **not** a UWP packaging test.

## Xbox One S input integration

The main frontend now merges Windows.Gaming.Input gamepad polling into the existing SDL PadState, including sticks and triggers (Windows Y is inverted to match SDL). This is source-level integration only: no Xbox One S UWP runtime or package test has passed. The existing UWP build, startup/windowing and game generation remain blocking tasks.
