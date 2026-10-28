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

## Xbox One S launch issue (diagnostic v0.1)

An Xbox One S Developer Mode test showed that the first signed UWP diagnostic APPX installed but immediately returned to the console dashboard without drawing the expected blue screen. A successful GitHub Actions compile/signing run **does not establish successful runtime launch**.

The v0.2 diagnostic makes the following changes:
- Moves Direct3D 11 device/swap-chain creation out of `IFrameworkView::Load` into `Run`, after `CoreWindow::Activate`.
- Checks and records the HRESULT of each Direct3D/DXGI setup stage and Present; if graphics fail, leaves the event loop active rather than closing the app.
- Catches gamepad-read WinRT exceptions independently, and records lifecycle callbacks including `wWinMain`, `Initialize`, `SetWindow`, `Load`, and `Run`.
- Writes a text log to **LocalState/ssrecomp-diagnostic.log** via the package's ApplicationData.LocalFolder path. If the executable never reaches `wWinMain`, the log may not be created; that points toward package activation or a missing runtime dependency.
- Bumps the package version to 0.2.0.0 and attempts to bundle the UWP x64 Microsoft.VCLibs package alongside the APPX when it exists on the GitHub runner. A new temporary test-signing certificate is made per workflow run; uninstall the previous diagnostic before installing a differently signed copy if updating fails.

### Retrieving the log on Xbox (after v0.2 test)

Open the Xbox Device Portal on a device on the same network as the Xbox, choose **File explorer**, navigate to **LocalAppData → SSRecomp.XboxUwpDiagnostic → LocalState**, and retrieve `ssrecomp-diagnostic.log`. It identifies the last successful stage and any failing `HRESULT` value. A log without `Run start` means launch or lifecycle was interrupted earlier. If the file does not exist, inspect portal crash reports and the Microsoft.VCLibs.140.00 prerequisite.

This is a developer diagnostic, not a SoulSilver game executable. No code change is considered confirmed working on Xbox until retested on the console.

## Xbox One S immediate exit: root cause confirmed by v0.2 LocalState log

The user's Xbox Device Portal diagnostic log contains four instances of:

```text
Diagnostic v0.2 | WinRT exception escaped CoreApplication | HRESULT=0x80010106
```

`0x80010106` is `RPC_E_CHANGED_MODE`: an attempt to change a COM apartment already initialized with a different concurrency mode. Because the log never included `wWinMain entered`, the most likely faulting call was the explicit `init_apartment(apartment_type::single_threaded)` executed immediately before that log in `wWinMain`. This was an application initialization issue, not a ROM failure.

Version **0.3.0.0** removes the forced COM apartment initialization and lets UWP's `CoreApplication::Run` manage its startup apartment, consistent with Microsoft's C++/WinRT Core App example. The standalone diagnostic workflow also checks that a future change does not reintroduce `init_apartment(` in the startup file. Package: `SSRecomp-Xbox-UWP-Diagnostic-v03.appx`, artifact: `xbox-uwp-diagnostic-v03-not-game`.

To retest, uninstall the older app (each GitHub run issues a distinct temporary self-signed signing certificate), install the versioned APPX in Xbox Developer Mode with the included UWP VCLibs x64 framework dependency, and launch. If it still exits, retrieve `LocalState/ssrecomp-diagnostic.log` via Xbox Device Portal and check for v0.3 log entries. Compiling successfully on Windows is **not** confirmation that startup works on Xbox.

## Xbox One S native SoulSilver game build track (separate from diagnostic)

**Important:** Successful installation of the diagnostic is not a playable SoulSilver build.

An Xbox One S hardware test proved that the UWP diagnostic v0.3 could start and draw colors.
Pressing B painted red and returned to the dashboard because Xbox maps B to
\`SystemNavigationManager.BackRequested\` in UWP apps. The diagnostic now calls
\`BackRequestedEventArgs.Handled(true)\` to keep B available for the DS cancel button.
This fix is in diagnostic v0.4 and still needs a console retest.

The actual game port has a separate build track:

1. The user's US SoulSilver ROM was verified *locally* against all 121 public code-section hashes. The game ROM itself and all generated game source code remain private.
2. \`tools/recomp.py --uwp\` now produces MSVC-compatible \`rc_image_###.cpp\` arrays instead of ELF/GAS \`.incbin\` assembly for the ROM code sections. The existing Linux/macOS/Android generator remains unchanged when \`--uwp\` is not used.
3. \`runtime/recomp_gen.h\` now supports MSVC branch hints and forced inlining.
4. The \`SSR_UWP\` CMake option handles MSVC's \`/bigobj\` and optimization flags, removes libpng screenshot dependency, and requires SDL2's WinRT non-XAML entry point for the game's existing SDL frontend.
5. \`frontend/main.cpp\` uses an app-local ROM at \`LocalState/rom.nds\` with save/config files in LocalState when launched via the Xbox/UWP app icon.
6. \`tools/uwp/build_game.ps1\` is a local-only build entry point. It verifies the ROM, generates the recompiled code, builds SDL2 for \`x64-uwp\` using vcpkg, then configures and links the game under \`CMAKE_SYSTEM_NAME=WindowsStore\`. Its output is not yet demonstrated to compile successfully end-to-end.
7. \`.github/workflows/xbox-uwp-core-build.yml\` compiles the actual melonDS static hardware core for UWP x64 **without any ROM**. The CMake WindowsStore version and unsupported directory-header probe were fixed after the first failed runs.

### Private native build from a Windows computer

\`\`\`powershell
.\tools\uwp\build_game.ps1 -Rom "C:\MyGames\SoulSilver.nds" -VcpkgRoot "C:\vcpkg"
\`\`\`

Prerequisites: Windows + Visual Studio 2022/2026 with UWP C++ and Windows SDK, Python 3,
CMake, Git, vcpkg and sufficient free RAM/disk for the 1.7 million generated ARM9
instructions. The script will NOT upload or publish the ROM or generated source.
A successful local link must still be followed by AppContainer packaging and
physical Xbox rendering/audio/input/save verification.

The next hardware milestone is **a native melonDS core library compiled for UWP x64**.
After that, the game/frontend must be linked with SDL2 WinRT and the user's
private generated code; packaging then must include the required SDL runtime and
app manifest. Do not describe a core-only or diagnostic APPX as the full game.

## Verified recompilation and widescreen compatibility (October 2026)

**Private user ROM recompilation:** all 121 SoulSilver US game code sections matched the reference layout. Running the UWP-aware recompiler locally generated 122 sections (including the built-in ARM9 BIOS), 25,120 chunks and 1,685,113 ARM9 instructions. There were 71 generated instruction source files and 122 C++ byte-image files, totaling about 368 MB of private sources. Code generation completed in about 10 seconds. A representative generated instruction translation unit, the game dispatch table, and a C++ code-image object compiled or passed syntax checks locally. The layout-based application metadata step yielded 59 application descriptors. These checks DO NOT prove a full Xbox game build.

**Widescreen/one-screen preservation:** The merged presentation and widened 3D renderer are already in the ssrecomp source. GPU3D_Wide.cpp previously used GCC-specific SIMD vector syntax unsupported by MSVC. ScalarWideVector.h supplies a tested four-lane fallback for Xbox MSVC, including numerical vector conversion and all-bits-set comparison masks. The original accelerated GCC/Clang path remains unchanged. The complete GPU3D_Wide.cpp passed an alternate local scalar-fallback C++20 syntax check; MSVC CI and real console frame rates remain unverified.

**Xbox B input:** The game's UWP controller reader now registers a SystemNavigationManager.BackRequested handler after the SDL window is created, so the console's Back action should not close the game on B. The game separately reads B for Nintendo DS Cancel. This is not yet hardware-tested with SDL2.

**ROM privacy and architecture:** Generated ARM9 code embeds code bytes from the user's private game ROM and is specific to that code layout. Only the open-source emulator/frontend and synthetic unit tests are committed. Do not upload private ROM, generated C++ or ROM-bearing EXE to public GitHub Actions. The user must compile their ROM-specific game executable privately, then place a compatible ROM file in the Xbox app's LocalState for cartridge assets. Simply dropping any ROM into the current diagnostic APPX cannot create a recompiled executable on the Xbox.

**Build gates still outstanding:** (1) melonDS UWP core.lib MSVC x64 compile; (2) SDL2 WinRT UWP static library and non-XAML entry point, separately probed via xbox-uwp-sdl2-probe.yml; (3) full private ROM-specific C++ game compilation/link; (4) separate game APPX packaging and dependencies; (5) Xbox One S tests for merged fullscreen, input, touch cursor, audio, saves and performance.

### Verified UWP native core milestone

GitHub Actions run 40619012771 **SUCCEEDED** on Visual Studio 2026 and Windows SDK 10.0.26100.0. MSBuild's log explicitly reports the UWP x64 static archive at build/uwp-core/src/Release/core.lib. This proves that melonDS hardware/graphics/audio sources, including the Xbox scalar widescreen fallback, compile under the WindowsStore toolchain. It does NOT prove the SDL frontend links or the Xbox console can boot the game.

The initial successful artifact only contained CMakeCache.txt because the workflow specified the wrong location for core.lib. The output path has been corrected in the workflow, and the next successful run should publish core.lib itself.

Local ROM-only recompilation metrics and MSVC alternatives were also tested: 1,685,113 ARM9 instructions generated across 25,120 chunks, 122 sections (121 ROM plus BIOS), 71 RC instruction C++ files and 122 private image C++ files, about 368 MB, plus 59 game application descriptors. These generated files were intentionally NOT uploaded to the GitHub repository.

A separate public SDL2 WinRT x64 build probe is now running in workflow xbox-uwp-sdl2-probe.yml to test the next native dependency without exposing private game data. The official SDL2 WinRT documentation requires its NonXAML main entry point and bundled cursor resources, both now referenced in the game CMake configuration.
