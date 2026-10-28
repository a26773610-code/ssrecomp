# Pack and sign a PRIVATE ROM-specific SoulSilver game for Xbox Developer Mode.
# Run after build_game.ps1. Do not upload this ROM-bearing executable to GitHub.
param(
    [string]$GameExe = "build\xbox-uwp-game\Release\ssrecomp.exe",
    [string]$OutputDir = "build\xbox-uwp-game\private-package"
)
$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
Set-Location $root
$exe = [IO.Path]::GetFullPath($GameExe)
if (-not (Test-Path $exe -PathType Leaf)) { throw "No native game executable found: $exe. Compile with your own ROM first." }
$destination = [IO.Path]::GetFullPath($OutputDir)
$stage = Join-Path $destination 'staging'
New-Item -ItemType Directory -Force $destination | Out-Null
if (Test-Path $stage) { Remove-Item $stage -Recurse -Force }
New-Item -ItemType Directory -Force $stage | Out-Null
Copy-Item $exe (Join-Path $stage 'ssrecomp.exe')
Copy-Item (Join-Path $root 'frontend\uwp\game_manifest.xml') (Join-Path $stage 'AppxManifest.xml')
Get-ChildItem (Split-Path $exe) -Filter '*.winmd' -File -ErrorAction SilentlyContinue | Copy-Item -Destination $stage
Get-ChildItem (Split-Path $exe) -Filter '*.dll' -File -ErrorAction SilentlyContinue | Copy-Item -Destination $stage
Add-Type -AssemblyName System.Drawing
foreach ($entry in @(@('Square44x44Logo.png',44,44),@('Square150x150Logo.png',150,150),@('StoreLogo.png',50,50),@('Wide310x150Logo.png',310,150))) {
    $name = [string]$entry[0]; $w = [int]$entry[1]; $h = [int]$entry[2]
    $bitmap = [Drawing.Bitmap]::new($w,$h)
    $graphics = [Drawing.Graphics]::FromImage($bitmap)
    $graphics.Clear([Drawing.Color]::FromArgb(18,36,61))
    $graphics.Dispose()
    $bitmap.Save((Join-Path $stage $name),[Drawing.Imaging.ImageFormat]::Png)
    $bitmap.Dispose()
}
$programFilesX86 = [Environment]::GetFolderPath('ProgramFilesX86')
$sdk = Join-Path $programFilesX86 'Windows Kits\10\bin'
if (-not (Test-Path $sdk)) { throw "Windows UWP SDK not available" }
$toolset = Get-ChildItem $sdk -Directory | Sort-Object Name -Descending | Where-Object { Test-Path (Join-Path $_.FullName 'x64\makeappx.exe') } | Select-Object -First 1
if (-not $toolset) { throw "UWP makeappx.exe missing" }
$makeappx = Join-Path $toolset.FullName 'x64\makeappx.exe'
$signtool = Join-Path $toolset.FullName 'x64\signtool.exe'
if (-not (Test-Path $signtool)) { throw "UWP signtool.exe missing" }
$appx = Join-Path $destination 'SSRecomp-Xbox-SoulSilver-Private.appx'
& $makeappx pack /d $stage /p $appx /o
if ($LASTEXITCODE -ne 0) { throw "Private SoulSilver APPX packaging failed" }
# Private certificate matching the game manifest's Publisher. No PFX is uploaded.
$cert = New-SelfSignedCertificate -Type Custom -Subject 'CN=SSRecompGame' -KeyUsage DigitalSignature -FriendlyName 'Private SoulSilver Xbox signer' -CertStoreLocation 'Cert:\CurrentUser\My' -KeyExportPolicy NonExportable -HashAlgorithm SHA256 -NotAfter (Get-Date).AddYears(1) -TextExtension @('2.5.29.37={text}1.3.6.1.5.5.7.3.3')
$cer = Join-Path $destination 'SSRecomp-Xbox-SoulSilver-Private.cer'
Export-Certificate -Cert $cert -FilePath $cer | Out-Null
& $signtool sign /fd SHA256 /sha1 $cert.Thumbprint /s My $appx
if ($LASTEXITCODE -ne 0) { throw "Private SoulSilver APPX signing failed" }
foreach ($location in @((Join-Path $programFilesX86 'Microsoft SDKs\Windows Kits\10\ExtensionSDKs\Microsoft.VCLibs'),(Join-Path $programFilesX86 'Windows Kits\10\ExtensionSDKs\Microsoft.VCLibs'))) {
    if (Test-Path $location) {
        $found = Get-ChildItem $location -Recurse -Filter 'Microsoft.VCLibs.x64.14.00.appx' -File -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($found) { Copy-Item $found.FullName $destination; break }
    }
}
Write-Host "Private ROM-derived game APPX: $appx"
Write-Host "Certificate: $cer"
Write-Host "Install into Xbox Developer Mode, then copy your compatible ROM to LocalState\rom.nds using Xbox Device Portal."
