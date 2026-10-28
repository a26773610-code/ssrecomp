$ErrorActionPreference = 'Stop'
$exe = Get-ChildItem build/game -Filter 'pokemon-ds-wide.exe' -Recurse | Where-Object { $_.FullName -match 'Release' } | Select-Object -First 1
if (-not $exe) { throw 'Complete frontend executable missing' }
Copy-Item $exe.FullName staging/pokemon-ds-wide.exe
Copy-Item tools/uwp/widescreen/AppxManifest.xml staging/AppxManifest.xml
Copy-Item switch-source/LICENSE staging/LICENSE.txt
Add-Type -AssemblyName System.Drawing
foreach ($entry in @(@('Square44x44Logo.png',44),@('Square150x150Logo.png',150),@('StoreLogo.png',50))) {
    $size = [int]$entry[1]
    $bitmap = [System.Drawing.Bitmap]::new($size,$size)
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    $graphics.Clear([System.Drawing.Color]::FromArgb(16,32,56))
    $pen = [System.Drawing.Pen]::new([System.Drawing.Color]::FromArgb(110,195,245), [Math]::Max(2,$size/20))
    $graphics.DrawRectangle($pen, [int]($size*.15), [int]($size*.28), [int]($size*.7), [int]($size*.44))
    $pen.Dispose(); $graphics.Dispose()
    $bitmap.Save((Join-Path (Resolve-Path staging) $entry[0]),[System.Drawing.Imaging.ImageFormat]::Png)
    $bitmap.Dispose()
}
New-Item -ItemType Directory -Force dist | Out-Null
Copy-Item tools/uwp/widescreen/README-PT.txt dist/LEIA-ME.txt
$sdk = Join-Path ${env:ProgramFiles(x86)} 'Windows Kits/10/bin'
$toolset = Get-ChildItem $sdk -Directory | Sort-Object Name -Descending | Where-Object { Test-Path (Join-Path $_.FullName 'x64/makeappx.exe') } | Select-Object -First 1
if (-not $toolset) { throw 'Windows SDK packaging tools missing' }
$makeappx = Join-Path $toolset.FullName 'x64/makeappx.exe'
$signtool = Join-Path $toolset.FullName 'x64/signtool.exe'
& $makeappx pack /d staging /p dist/PokemonDS-Widescreen-Xbox.appx /o
if ($LASTEXITCODE) { throw 'APPX packaging failed' }
$cert = New-SelfSignedCertificate -Type Custom -Subject 'CN=PokemonDSWidescreen' -KeyUsage DigitalSignature -FriendlyName 'Pokemon DS Widescreen test build' -CertStoreLocation 'Cert:/CurrentUser/My' -KeyExportPolicy NonExportable -HashAlgorithm SHA256 -NotAfter (Get-Date).AddYears(1) -TextExtension @('2.5.29.37={text}1.3.6.1.5.5.7.3.3')
Export-Certificate -Cert $cert -FilePath dist/PokemonDS-Widescreen-Xbox.cer | Out-Null
& $signtool sign /fd SHA256 /sha1 $cert.Thumbprint /s My dist/PokemonDS-Widescreen-Xbox.appx
if ($LASTEXITCODE) { throw 'APPX signing failed' }
# The developer certificate is self-signed; trust is configured on the test device.
Add-Type -AssemblyName System.IO.Compression.FileSystem
$zip = [System.IO.Compression.ZipFile]::OpenRead((Resolve-Path dist/PokemonDS-Widescreen-Xbox.appx))
try {
    foreach ($name in @('AppxManifest.xml','pokemon-ds-wide.exe','funcs/IREO.txt')) {
        if (-not ($zip.Entries | Where-Object { $_.FullName.Replace('\','/') -eq $name })) { throw "Missing package member: $name" }
    }
    if ($zip.Entries | Where-Object { $_.FullName -match '\.(nds|sav|pfx)$' }) { throw 'Unexpected private data in package' }
} finally { $zip.Dispose() }
$roots = @((Join-Path ${env:ProgramFiles(x86)} 'Microsoft SDKs/Windows Kits/10/ExtensionSDKs/Microsoft.VCLibs'), (Join-Path ${env:ProgramFiles(x86)} 'Windows Kits/10/ExtensionSDKs/Microsoft.VCLibs'))
foreach ($root in $roots) {
    if (Test-Path $root) {
        $dep = Get-ChildItem $root -Filter Microsoft.VCLibs.x64.14.00.appx -Recurse | Select-Object -First 1
        if ($dep) { Copy-Item $dep.FullName dist/; break }
    }
}
Get-FileHash dist/*.appx -Algorithm SHA256 | Format-List | Out-File dist/SHA256.txt
