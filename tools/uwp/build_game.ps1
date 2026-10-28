# Private SoulSilver native recompilation for Windows Store / Xbox One S UWP.
# This script runs ONLY on the user's trusted Windows build machine.
# Do not upload ROM, gen/, compiled game EXE or private keys to public GitHub.
param(
    [Parameter(Mandatory = $true)][string]$Rom,
    [string]$VcpkgRoot = "",
    [string]$SDL2SourceRoot = "",
    [int]$Jobs = 4
)
$ErrorActionPreference = 'Stop'
$root = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
Set-Location $root
$romPath = [System.IO.Path]::GetFullPath($Rom)
if (-not (Test-Path -LiteralPath $romPath -PathType Leaf)) {
    throw "Missing user-owned SoulSilver ROM: $romPath"
}
# SDL2 is compiled from verified public WinRT sources in the same CMake
# invocation as the game. No vcpkg or desktop SDL2 binaries are needed.
if (-not (Get-Command python -ErrorAction SilentlyContinue)) { throw "Python 3 not found" }

if ([string]::IsNullOrWhiteSpace($SDL2SourceRoot)) {
    $SDL2SourceRoot = Join-Path $root '.private-uwp-deps\SDL2'
    $winrtMain = Join-Path $SDL2SourceRoot 'src\main\winrt\SDL_winrt_main_NonXAML.cpp'
    if (-not (Test-Path $winrtMain)) {
        New-Item -ItemType Directory -Force (Split-Path $SDL2SourceRoot -Parent) | Out-Null
        git clone --depth 1 --branch release-2.32.10 https://github.com/libsdl-org/SDL.git $SDL2SourceRoot
        if ($LASTEXITCODE -ne 0) { throw "SDL2 WinRT source download failed" }
    }
}
$winrtMain = Join-Path $SDL2SourceRoot 'src\main\winrt\SDL_winrt_main_NonXAML.cpp'
if (-not (Test-Path $winrtMain)) { throw "Missing SDL2 UWP entry point: $winrtMain" }

Write-Host "Verify SoulSilver US ROM locally (121 code sections; never uploaded)"
python tools/uwp/verify_soulsilver_code.py $romPath
if ($LASTEXITCODE -ne 0) { throw "ROM code does not match SoulSilver US layout" }

Write-Host "Generate native ARM9 C++ with MSVC image arrays into private gen/"
python tools/recomp.py melonDS/src gen --rom $romPath --layout layouts/soulsilver-us.json --uwp
if ($LASTEXITCODE -ne 0) { throw "Recompiler failed" }
python tools/apps.py --layout layouts/soulsilver-us.json gen
if ($LASTEXITCODE -ne 0) { throw "App table generation failed" }

Write-Host "SDL2 WinRT will be compiled statically from the public SDL2 source tree."

$generatorOutput = [string]::Join([Environment]::NewLine, @(cmake --help))
if ($generatorOutput -match 'Visual Studio 18 2026') {
    $generator = 'Visual Studio 18 2026'
} elseif ($generatorOutput -match 'Visual Studio 17 2022') {
    $generator = 'Visual Studio 17 2022'
} else {
    throw "Visual Studio 2022 or 2026 CMake generator missing."
}

$cmakeArgs = @(
    '-S', '.', '-B', 'build/xbox-uwp-game', '-G', $generator, '-A', 'x64',
    '-DCMAKE_SYSTEM_NAME=WindowsStore',
    '-DCMAKE_SYSTEM_VERSION=10.0',
    '-DCMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION=10.0.26100.0',
    '-DCMAKE_VS_WINDOWS_TARGET_PLATFORM_MIN_VERSION=10.0.17763.0',
    '-DSSR_UWP=ON',
    "-DSSR_SDL2_WINRT_SOURCE_DIR=$SDL2SourceRoot",
    "-DSSR_GENERATED_DIR=$root\gen",
    '-DENABLE_JIT=OFF', '-DENABLE_OGLRENDERER=OFF'
)
Write-Host "Configure full private UWP SoulSilver game project"
& cmake @cmakeArgs
if ($LASTEXITCODE -ne 0) { throw "UWP SoulSilver CMake configuration failed" }

Write-Host "Compile and link complete game frontend and generated ARM9 (private)"
cmake --build build/xbox-uwp-game --config Release --target ssrecomp --parallel $Jobs
if ($LASTEXITCODE -ne 0) { throw "Full SoulSilver UWP native game build failed" }

Write-Host "Native UWP x64 SoulSilver game linked. Packaging and Xbox hardware testing remain necessary."
Write-Host "KEEP gen/ and build/xbox-uwp-game PRIVATE. They contain code from your ROM."
