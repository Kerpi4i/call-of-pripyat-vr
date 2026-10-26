param([switch]$SkipNativeBuild)
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
if(!$SkipNativeBuild -and (Get-Process xrEngine -ErrorAction SilentlyContinue)){throw 'Close xrEngine before a native release build.'}
if(!$SkipNativeBuild){
    & (Join-Path $PSScriptRoot 'build-bridge.ps1')
    & (Join-Path $PSScriptRoot 'build-engine.ps1') -Target Game
}
$version='0.1.0-alpha.2'
$stage=Join-Path $root "build\release-$version"
if(Test-Path -LiteralPath $stage){throw "Staging directory exists; preserve it or select a new version: $stage"}
New-Item -ItemType Directory -Force $stage,(Join-Path $root 'dist') | Out-Null
foreach($name in @('gamedata','defaults','docs','licenses')){Copy-Item -LiteralPath (Join-Path $root "release\$name") -Destination $stage -Recurse}
foreach($name in @('Uninstall.ps1','asset-manifest.json','CoPVR.ico')){Copy-Item -LiteralPath (Join-Path $root "release\$name") -Destination $stage}
foreach($name in @('LICENSE','THIRD-PARTY-NOTICES.md')){Copy-Item -LiteralPath (Join-Path $root $name) -Destination $stage}
$bin=Join-Path $stage 'bin';New-Item -ItemType Directory $bin|Out-Null
# Only Windows x64 runtime files. The executable also imports the GL DLL at load time,
# even though the VR launcher selects R4; keep that bootstrap dependency.
$native=Join-Path $root 'engine\bin\x64\Release'
$excluded=@('AnselSDK64.dll','discord_game_sdk.dll','concrt140.dll','vcamp140.dll','vccorlib140.dll','msvcp140_codecvt_ids.dll')
$files=Get-ChildItem -LiteralPath $native -File | Where-Object { $_.Extension -eq '.dll' -and $_.Name -notin $excluded }
foreach($file in $files){Copy-Item -LiteralPath $file.FullName -Destination $bin}
Copy-Item -LiteralPath (Join-Path $native 'xrEngine.exe') -Destination $bin
$compiler=Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe'
$common=Join-Path $root 'release\launcher\Common.cs'
$manifest=Join-Path $root 'release\app.manifest'
$refs=@('/r:System.Windows.Forms.dll','/r:System.Drawing.dll','/r:System.Core.dll','/r:System.IO.Compression.dll','/r:System.IO.Compression.FileSystem.dll')
& $compiler /nologo /target:winexe /platform:x64 /optimize+ "/win32manifest:$manifest" "/win32icon:$root\release\CoPVR.ico" @refs "/out:$stage\CoPVR.exe" $common (Join-Path $root 'release\launcher\Launcher.cs') (Join-Path $root 'release\launcher\AssemblyInfo.cs')
if($LASTEXITCODE-ne 0){throw 'Launcher compilation failed'}
@('<?xml version="1.0"?>','<configuration><startup><supportedRuntime version="v4.0" sku=".NETFramework,Version=v4.8"/></startup></configuration>') | Set-Content -LiteralPath (Join-Path $stage 'CoPVR.exe.config') -Encoding utf8
$managed=@(Get-ChildItem -LiteralPath $stage -Recurse -File | ForEach-Object {$_.FullName.Substring($stage.Length+1)})
$managed | Set-Content -LiteralPath (Join-Path $stage 'installed-files.txt') -Encoding utf8
$integrity=Get-ChildItem -LiteralPath $stage -Recurse -File | ForEach-Object { (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant()+'  '+$_.FullName.Substring($stage.Length+1) }
$integrity | Set-Content -LiteralPath (Join-Path $stage 'integrity.sha256') -Encoding ascii
$payload=Join-Path $root "dist\CoPVR-$version-portable.zip"
if(Test-Path -LiteralPath $payload){throw 'Release ZIP already exists; refusing overwrite.'}
Add-Type -AssemblyName System.IO.Compression.FileSystem
[IO.Compression.ZipFile]::CreateFromDirectory($stage,$payload,[IO.Compression.CompressionLevel]::Optimal,$false)
$setup=Join-Path $root "dist\CoPVR-$version-Setup.exe"
& $compiler /nologo /target:winexe /platform:x64 /optimize+ "/win32manifest:$manifest" "/win32icon:$root\release\CoPVR.ico" @refs "/resource:$payload,payload.zip" "/out:$setup" $common (Join-Path $root 'release\launcher\Setup.cs') (Join-Path $root 'release\launcher\AssemblyInfo.cs')
if($LASTEXITCODE-ne 0){throw 'Installer compilation failed'}
Copy-Item -LiteralPath (Join-Path $stage 'CoPVR.exe.config') -Destination ($setup+'.config')
& (Join-Path $root 'tools\toolchain\zig-x86_64-windows-0.15.2\zig.exe') version | Out-Null
& git -C (Join-Path $root 'engine') diff --binary --output=../patches/openxray-vr.patch
& python (Join-Path $PSScriptRoot 'package-source.py')
if($LASTEXITCODE -ne 0){throw 'Source archive failed'}
$hashes=Get-ChildItem -LiteralPath (Join-Path $root 'dist') -File | Where-Object { $_.Name -like "CoPVR-$version-*" -or $_.Name -eq "VALIDATION-$version.md" } | ForEach-Object { (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant()+'  '+$_.Name }
$hashes | Set-Content -LiteralPath (Join-Path $root "dist\SHA256SUMS-$version.txt") -Encoding ascii
Write-Output "Release candidate prepared: $setup"
Write-Output 'No installer, launcher or game was started. Clean installation/headset acceptance is still required before publication.'
