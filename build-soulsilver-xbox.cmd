@echo off
setlocal
rem EXPERIMENTAL PRIVATE BUILD: drag your own compatible SoulSilver ROM onto this file.
rem Requires Windows, Visual Studio C++ UWP, Windows SDK, Git, CMake, Python 3.
rem The native UWP game frontend is still under development; do not expect every
rem build to succeed before the integration test is approved.
if "%~1"=="" (
  echo Drag your legally obtained SoulSilver .nds ROM onto this script.
  echo Or run: build-soulsilver-xbox.cmd "C:\MyGames\SoulSilver.nds"
  pause
  exit /b 2
)
echo Starting private SoulSilver UWP x64 game build from your ROM.
echo ROM files and generated source remain on this computer.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\uwp\build_game.ps1" -Rom "%~1"
set "BUILD_RESULT=%ERRORLEVEL%"
if not "%BUILD_RESULT%"=="0" (
  echo Game build was not successful. Read the PowerShell error above.
  pause
  exit /b %BUILD_RESULT%
)
echo Your signed Xbox Developer Mode game APPX is under build\xbox-uwp-game\private-package.
pause
exit /b 0
