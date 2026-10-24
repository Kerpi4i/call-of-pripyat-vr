$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$zig = Join-Path $PSScriptRoot 'toolchain\zig-x86_64-windows-0.15.2\zig.exe'
& (Join-Path $PSScriptRoot 'build-portable.ps1')
if (!(Test-Path -LiteralPath (Join-Path $PSScriptRoot 'openxr-loader\x64\bin\openxr_loader.dll'))) {
    & (Join-Path $PSScriptRoot 'bootstrap-openxr.ps1')
}
$flags = @('-std=c++17','-Wall','-Wextra','-Wno-missing-field-initializers','-Wno-cast-function-type-mismatch',"-I$root\src","-I$root\third_party\openxr\include")
& $zig c++ @flags "$root\tests\stereo_tests.cpp" -o "$root\build\stereo_tests.exe"
if ($LASTEXITCODE -ne 0) { throw 'Stereo test build failed' }
& "$root\build\stereo_tests.exe"
if ($LASTEXITCODE -ne 0) { throw 'Stereo tests failed' }
& $zig c++ @flags "$root\src\vr_diagnostic.cpp" "$root\src\openxr_input.cpp" -ld3d11 -ldxgi -ld3dcompiler_47 -luser32 -o "$root\build\vr_diagnostic.exe"
if ($LASTEXITCODE -ne 0) { throw 'VR diagnostic build failed' }
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'openxr-loader\x64\bin\openxr_loader.dll') -Destination (Join-Path $root 'build\openxr_loader.dll') -Force
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'openxr-loader\share\doc\openxr\LICENSE') -Destination (Join-Path $root 'build\OPENXR-LOADER-LICENSE.txt') -Force
Write-Output 'VR diagnostic ready. Use tools/run-vr.ps1 -Probe or tools/run-vr.ps1 -Seconds 60.'
