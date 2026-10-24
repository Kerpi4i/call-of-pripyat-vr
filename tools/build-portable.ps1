$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$zig = Join-Path $PSScriptRoot 'toolchain\zig-x86_64-windows-0.15.2\zig.exe'
if (!(Test-Path -LiteralPath $zig)) { throw 'Portable Zig 0.15.2 is missing; use tools/build.ps1 with Windows SDK or download the pinned toolchain.' }
$build = Join-Path $root 'build'
New-Item -ItemType Directory -Force $build | Out-Null
& $zig c++ -std=c++17 -Wall -Wextra -Wno-missing-field-initializers "-I$root\src" "$root\tests\interaction_tests.cpp" -o "$build\interaction_tests.exe"
if ($LASTEXITCODE -ne 0) { throw 'Test build failed' }
& $zig c++ -std=c++17 -Wall -Wextra -Wno-missing-field-initializers "-I$root\src" "-I$root\third_party\openxr\include" -c "$root\src\openxr_input.cpp" -o "$build\openxr_input.obj"
if ($LASTEXITCODE -ne 0) { throw 'OpenXR input build failed' }
& $zig ar rcs "$build\libcopvr_input.a" "$build\openxr_input.obj"
if ($LASTEXITCODE -ne 0) { throw 'Archive failed' }
& "$build\interaction_tests.exe"
if ($LASTEXITCODE -ne 0) { throw 'Tests failed' }
