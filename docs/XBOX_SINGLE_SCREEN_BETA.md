# Xbox One S — SSRecomp single-screen beta test

**What this is:** A ROM-free Xbox UWP x64 package containing the real ssrecomp
frontend, melonDS core, SDL2 WinRT video/audio/input, and the `merged`
single-screen presentation. For this first hardware test it deliberately runs
ARM9 **on the melonDS interpreter** (`--interp`), so it is **not the final
SoulSilver static-recompilation build**. Real-frame-rate performance, merged
scene transitions and input are unverified on the Xbox One S.

**What this is not:** The successful ROM-free link-probe APPX from earlier
runs has dummy instruction tables and is not a native-recompiled game.
The interpreter beta is a distinct build that explicitly disables those
dummy recompilation instructions to try reading the real user-owned ROM.

## Installing on an Xbox One S in Developer Mode

1. Download the artifact named
   `xbox-one-s-ssrecomp-single-screen-interpreter-beta-no-rom` from the
   `xbox-uwp-interpreter-beta.yml` GitHub Actions workflow. Only use a run
   that reports **Success**.
2. Extract the ZIP. The artifact contains
   `SSRecomp-Xbox-SingleScreen-InterpreterBeta-v01.appx`, the signing
   certificate `SSRecomp-Xbox-InterpreterBeta.cer`, and, if the SDK supplied
   it, a `Microsoft.VCLibs.x64.14.00.appx` dependency package.
3. Open the Xbox Device Portal in a browser connected to the same network as
   your Developer Mode console. Under the application installer, add the APPX
   and the certificate, plus the VCLibs dependency if requested.
4. Launch **SSRecomp Single Screen Beta**. Without a game ROM, it should stay
   open and display **ROM NOT FOUND** rather than closing.
5. In Xbox Device Portal's File Explorer, find the local data directory of
   the **SSRecomp.XboxInterpreterBeta** package, enter `LocalState`, then
   upload your **own compatible SoulSilver ROM** renamed exactly `rom.nds`.
   It must be `LocalState/rom.nds` for *this package identity*, not the
   previous diagnostic app's LocalState.
6. The beta is designed to notice `rom.nds` and start. If the app returns to
   the dashboard or does not move past the setup screen, close and relaunch
   the app. Retrieve `LocalState/ssrecomp-game.log` through Device Portal
   and share the log text for debugging.

The user-provided European `Pokemon - SoulSilver Version (v10) (E).nds`
was verified locally against the project's 121 game code sections and is
a suitable input for compatibility testing. Another separately provided
`HG-Golden-Edition-2.0-(PTBR).NDS` did **not** pass that validation and must
not be assumed compatible.

## What to verify on real hardware

- The app **stays open** with no ROM and shows the setup instructions.
- After copying the compatible ROM, the melonDS core boots without a crash.
- The interface uses one television picture and the `merged` layout.
  A full 16:9 extended world and all special menus remain **unverified**
  because the interpreter does not currently emit the recompiled game's
  scene/function probes.
- The Xbox's **B** button works as DS Cancel rather than closing the app.
- D-pad, A, X, Y, sticks, sound and save writes work.
- `LocalState/soulsilver.sav` is created/updated and the game continues
  across launches. Preserve this file when installing a new build.

## Long-term native-recompiled build

The original goal remains an x64 UWP APPX containing ARM9 game code statically
recompiled from the player's own ROM with `tools/recomp.py --uwp`, retaining
the same ssrecomp single-screen frontend. Those ~368 MB of generated private
C++ sources **cannot** be recreated by copying a ROM into the beta after
installation. They need to be compiled on a trusted Windows build machine,
or securely supplied to a private build environment, then signed into a
different ROM-specific app package.

The private build entrypoint is
`tools/uwp/build_game.ps1 -Rom "C:\MyGames\SoulSilver.nds"`. Its output
has not yet been validated end-to-end or installed on Xbox hardware.

Never upload copyrighted game ROMs or ROM-derived source and binaries to
the project's public GitHub Actions logs or artifacts.
