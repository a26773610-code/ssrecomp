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
- [x] Connect polled buttons and analog axes to existing frontend PadState (source integration)
- [ ] Validate native Xbox input on hardware
- [x] Add Windows-hosted MSVC desktop and UWP-macro helper compile smoke checks (CI results not verified)
- [ ] Verify CI results and build with actual UWP SDK/toolchain
- [ ] Test installation and runtime in Xbox Developer Mode

No functional Xbox build is available yet. The helper is included in CMake and referenced by melonDS local-file functions, but UWP execution has not been compiled or tested. Do not commit copyrighted game ROMs, BIOS files or private signing keys.

## Distribution

A build that runs in Developer Mode does not imply that it can be installed in Retail Mode. Retail distribution requires an authorized Microsoft-supported route and is not guaranteed for new UWP games.

## Build blocker: generated game sources

The repository does not include `gen/sources.cmake`, which the root CMakeLists.txt includes. The game cannot be built from the public source checkout alone. The existing generation workflow must be reviewed and run with a legally obtained ROM outside the public repository. Never upload ROMs or derived copyrighted game data to public GitHub Actions artifacts.

The gamepad reader polls the first UWP Gamepad and merges its input into the existing frontend frame input. The Windows CI smoke test compiles helper translation units under UWP macros, but is **not** a UWP application packaging or runtime test.

## Xbox One S input integration

The main frontend now merges Windows.Gaming.Input gamepad polling into the existing SDL PadState, including sticks and triggers (Windows Y is inverted to match SDL). This is source-level integration only: no Xbox One S UWP runtime or package test has passed. The existing UWP build, startup/windowing and game generation remain blocking tasks.

## CI improvements

Windows smoke checks now locate the installed MSVC toolchain with vswhere rather than assuming the Visual Studio Enterprise path. The workflow compiles both ordinary Windows fallback branches and UWP-specific C++/WinRT storage/controller branches. These checks must still be confirmed by a green GitHub Actions run; compiling helpers alone does not establish that melonDS, SDL2 or the generated game can build for Xbox.

## Validation progress — Xbox One S

- The user's private US SoulSilver ROM was checked locally against the repository's public `layouts/soulsilver-us.json` reference: **all 121 code section SHA-1 digests and lengths matched**, including ARM9, ITCM, and overlays. The full ROM hash differs, so this is a code compatibility conclusion only, not proof that all resources are identical.
- Added `tools/uwp/verify_soulsilver_code.py` to repeat the section check on any local machine without copying/extracting ROM data to GitHub.
- `frontend/uwp/diagnostic_main.cpp` is a native UWP CoreWindow/Direct3D 11 diagnostic that paints blue and changes colors when pressing Xbox A/B/X/Y. It is **not the Pokémon game**.
- The standalone diagnostic package workflow is intended to compile an x64 AppContainer EXE, pack an APPX, sign it using a temporary certificate and upload only the diagnostic APPX plus public certificate. C++ CRT must use `/MD` rather than unsupported UWP `/MT`; the manifest declares the Microsoft.VCLibs UWP framework dependency.
- CI runs are triggered via draft PR [#1](https://github.com/a26773610-code/ssrecomp/pull/1). Their success and actual runtime behavior still need verification; a green compile will not itself mean the game can launch.
- Next prerequisite for an actual SoulSilver executable: locally generate `gen/` from a privately held ROM and compile the SDL2/melonDS/game frontend with an Xbox-compatible UWP entrypoint and graphics/audio surface. Do not publish generated game code or ROM as CI artifacts.
