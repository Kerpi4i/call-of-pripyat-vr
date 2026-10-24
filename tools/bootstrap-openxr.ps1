$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$destination = Join-Path $root 'tools\openxr-loader'
$archive = Join-Path $destination 'loader.zip'
New-Item -ItemType Directory -Force $destination | Out-Null
if (!(Test-Path -LiteralPath $archive)) {
    Invoke-WebRequest 'https://github.com/KhronosGroup/OpenXR-SDK-Source/releases/download/release-1.1.65/openxr_loader_windows-1.1.65.zip' -OutFile $archive
}
$expected = '859d7de35b7a968112db770b09ed8bddeed3cb16fd8ae070643a857676bea6fd'
if ((Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash.ToLowerInvariant() -ne $expected) { throw 'OpenXR loader checksum mismatch' }
Expand-Archive -LiteralPath $archive -DestinationPath $destination -Force
Write-Output 'Official Khronos OpenXR loader 1.1.65 extracted locally.'
