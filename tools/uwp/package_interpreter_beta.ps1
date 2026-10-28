# ROM-FREE Xbox Developer Mode beta: actual ssrecomp merged frontend + melonDS
# interpreter. This does NOT contain native-recompiled SoulSilver game code.
param(
    [Parameter(Mandatory = $true)][string]$GameExe,
    [string]$OutputDir = "build\interpreter-beta-package"
)
$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
Set-Location $root
$exe = [IO.Path]::GetFullPath($GameExe)
if (-not (Test-Path -LiteralPath $exe -PathType Leaf)) { throw "UWP frontend EXE missing: $exe" }
$destination = [IO.Path]::GetFullPath($OutputDir)
$stage = Join-Path $destination 'staging'
if (Test-Path $stage) { Remove-Item $stage -Recurse -Force }
New-Item -ItemType Directory -Force $stage | Out-Null
$manifest = Join-Path $root 'frontend\uwp\interpreter_manifest.xml'
[xml]$xml = Get-Content -LiteralPath $manifest -Raw
if ($xml.Package.Identity.Name -ne 'SSRecomp.XboxInterpreterBeta') { throw "Interpreter package identity invalid" }
if ($xml.Package.Identity.ProcessorArchitecture -ne 'x64') { throw "Interpreter must be x64" }
Copy-Item $exe (Join-Path $stage 'ssrecomp.exe')
Copy-Item $manifest (Join-Path $stage 'AppxManifest.xml')
Get-ChildItem (Split-Path $exe) -Filter '*.winmd' -File -ErrorAction SilentlyContinue | Copy-Item -Destination $stage
Get-ChildItem (Split-Path $exe) -Filter '*.dll' -File -ErrorAction SilentlyContinue | Copy-Item -Destination $stage
Add-Type -AssemblyName System.Drawing
foreach ($entry in @(
    @('Square44x44Logo.png',44,44),
    @('Square150x150Logo.png',150,150),
    @('StoreLogo.png',50,50),
    @('Wide310x150Logo.png',310,150)
)) {
    $name = [string]$entry[0]; $width = [int]$entry[1]; $height = [int]$entry[2]
    $bitmap = [Drawing.Bitmap]::new($width,$height)
    $graphics = [Drawing.Graphics]::FromImage($bitmap)
    $graphics.Clear([Drawing.Color]::FromArgb(13,26,48))
    $graphics.Dispose()
    $bitmap.Save((Join-Path $stage $name), [Drawing.Imaging.ImageFormat]::Png)
    $bitmap.Dispose()
}
$programFilesX86 = [Environment]::GetFolderPath('ProgramFilesX86')
$sdk = Join-Path $programFilesX86 'Windows Kits\10\bin'
$toolset = Get-ChildItem $sdk -Directory | Sort-Object Name -Descending | Where-Object {
    Test-Path (Join-Path $_.FullName 'x64\makeappx.exe')
} | Select-Object -First 1
if (-not $toolset) { throw "UWP makeappx tool unavailable" }
$makeappx = Join-Path $toolset.FullName 'x64\makeappx.exe'
$signtool = Join-Path $toolset.FullName 'x64\signtool.exe'
$appx = Join-Path $destination 'SSRecomp-Xbox-SingleScreen-InterpreterBeta-v02.appx'
& $makeappx pack /d $stage /p $appx /o
if ($LASTEXITCODE -ne 0) { throw "Interpreter APPX packaging failed" }
$cert = New-SelfSignedCertificate -Type Custom -Subject 'CN=SSRecompInterpreter' -KeyUsage DigitalSignature -FriendlyName 'SSRecomp Xbox ROM-free beta test cert' -CertStoreLocation 'Cert:\CurrentUser\My' -KeyExportPolicy NonExportable -HashAlgorithm SHA256 -NotAfter (Get-Date).AddDays(90) -TextExtension @('2.5.29.37={text}1.3.6.1.5.5.7.3.3')
$cer = Join-Path $destination 'SSRecomp-Xbox-InterpreterBeta.cer'
Export-Certificate -Cert $cert -FilePath $cer | Out-Null
& $signtool sign /fd SHA256 /sha1 $cert.Thumbprint /s My $appx
if ($LASTEXITCODE -ne 0) { throw "Interpreter APPX signing failed" }
foreach ($dir in @(
    (Join-Path $programFilesX86 'Microsoft SDKs\Windows Kits\10\ExtensionSDKs\Microsoft.VCLibs'),
    (Join-Path $programFilesX86 'Windows Kits\10\ExtensionSDKs\Microsoft.VCLibs')
)) {
    if (-not (Test-Path $dir)) { continue }
    $found = Get-ChildItem $dir -Filter 'Microsoft.VCLibs.x64.14.00.appx' -Recurse -File -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($found) { Copy-Item $found.FullName $destination; break }
}
if (Get-ChildItem $destination -Include *.nds,*.NDS -File -Recurse -ErrorAction SilentlyContinue) {
    throw "Refusing to publish a game ROM"
}
Write-Host "Interpreter beta package: $appx"
Write-Host "Signing certificate: $cer"
Write-Host "ROM-free beta only. Copy your own ROM into LocalState\rom.nds."
