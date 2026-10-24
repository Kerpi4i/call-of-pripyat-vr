param([switch]$BuildOnly)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$zig = Join-Path $PSScriptRoot 'toolchain\zig-x86_64-windows-0.15.2\zig.exe'
& $zig c++ -std=c++17 -O2 -Wall -Wextra -Wno-missing-field-initializers -Wno-cast-function-type-mismatch "-I$root\src" "-I$root\third_party\openxr\include" -shared "$root\src\vr_bridge.cpp" "$root\src\openxr_input.cpp" -ld3d11 -ldxgi -o "$root\build\copvr_bridge.dll"
if ($LASTEXITCODE -ne 0) { throw 'VR bridge build failed' }
$destination = Join-Path $root 'engine\bin\x64\Release'
if (!$BuildOnly) {
& $zig c++ -std=c++17 -O2 -Wall -Wextra "-I$root\src" "$root\tests\body_tracking_tests.cpp" -o "$root\build\body_tracking_tests.exe"
if ($LASTEXITCODE -ne 0) { throw 'Body tracking test build failed' }
& "$root\build\body_tracking_tests.exe"
if ($LASTEXITCODE -ne 0) { throw 'Body tracking tests failed' }
& $zig c++ -std=c++17 -O2 -Wall -Wextra -Wno-cast-function-type-mismatch "-I$root\src" "$root\tests\bridge_abi_tests.cpp" -o "$root\build\bridge_abi_tests.exe"
if ($LASTEXITCODE -ne 0) { throw 'Bridge ABI test build failed' }
& "$root\build\bridge_abi_tests.exe"
if ($LASTEXITCODE -ne 0) { throw 'Bridge ABI tests failed' }
& $zig c++ -std=c++17 -O2 -Wall -Wextra "-I$root\src" "$root\tests\trigger_gate_tests.cpp" -o "$root\build\trigger_gate_tests.exe"
if ($LASTEXITCODE -ne 0) { throw 'Trigger test build failed' }
& "$root\build\trigger_gate_tests.exe"
if ($LASTEXITCODE -ne 0) { throw 'Trigger tests failed' }
$destination = Join-Path $root 'engine\bin\x64\Release'
& $zig c++ -std=c++17 -O2 -Wall -Wextra "-I$root\src" "$root\tests\ui_ray_tests.cpp" -o "$root\build\ui_ray_tests.exe"
if ($LASTEXITCODE -ne 0) { throw 'UI ray test build failed' }
& "$root\build\ui_ray_tests.exe"
if ($LASTEXITCODE -ne 0) { throw 'UI ray tests failed' }
& $zig c++ -std=c++17 -O2 -Wall -Wextra "-I$root\src" "$root\tests\beam_math_tests.cpp" -o "$root\build\beam_math_tests.exe"
if ($LASTEXITCODE -ne 0) { throw 'Beam math test build failed' }
& "$root\build\beam_math_tests.exe"
if ($LASTEXITCODE -ne 0) { throw 'Beam math tests failed' }
& $zig c++ -std=c++17 -O2 -Wall -Wextra "-I$root\src" "$root\tests\arm_ik_tests.cpp" -o "$root\build\arm_ik_tests.exe"
if ($LASTEXITCODE -ne 0) { throw 'Arm IK test build failed' }
& "$root\build\arm_ik_tests.exe"
if ($LASTEXITCODE -ne 0) { throw 'Arm IK tests failed' }
& $zig c++ -std=c++17 -O2 -Wall -Wextra "-I$root\src" "$root\tests\hand_symmetry_tests.cpp" -o "$root\build\hand_symmetry_tests.exe"
if ($LASTEXITCODE -ne 0) { throw 'Hand symmetry test build failed' }
& "$root\build\hand_symmetry_tests.exe"
if ($LASTEXITCODE -ne 0) { throw 'Hand symmetry tests failed' }
& $zig c++ -std=c++17 -O2 -Wall -Wextra "-I$root\src" "$root\tests\navigation_gate_tests.cpp" -o "$root\build\navigation_gate_tests.exe"
if ($LASTEXITCODE -ne 0) { throw 'Navigation gate test build failed' }
& "$root\build\navigation_gate_tests.exe"
if ($LASTEXITCODE -ne 0) { throw 'Navigation gate tests failed' }
& $zig c++ -std=c++17 -O2 -Wall -Wextra "-I$root\src" "$root\tests\projection_decompression_tests.cpp" -o "$root\build\projection_decompression_tests.exe"
if ($LASTEXITCODE -ne 0) { throw 'Projection decompression test build failed' }
& "$root\build\projection_decompression_tests.exe"
if ($LASTEXITCODE -ne 0) { throw 'Projection decompression tests failed' }
& $zig c++ -std=c++17 -O2 -Wall -Wextra "-I$root\src" "$root\tests\two_hand_tests.cpp" -o "$root\build\two_hand_tests.exe"
if ($LASTEXITCODE -ne 0) { throw 'Two-hand test build failed' }
& "$root\build\two_hand_tests.exe"
if ($LASTEXITCODE -ne 0) { throw 'Two-hand tests failed' }
& $zig c++ -std=c++17 -O2 -Wall -Wextra "-I$root\src" "$root\tests\weapon_contact_tests.cpp" -o "$root\build\weapon_contact_tests.exe"
if ($LASTEXITCODE -ne 0) { throw 'Weapon contact test build failed' }
& "$root\build\weapon_contact_tests.exe"
if ($LASTEXITCODE -ne 0) { throw 'Weapon contact tests failed' }
& $zig c++ -std=c++17 -O2 -Wall -Wextra "-I$root\src" "$root\tests\grenade_primer_tests.cpp" -o "$root\build\grenade_primer_tests.exe"
if ($LASTEXITCODE -ne 0) { throw 'Grenade primer test build failed' }
& "$root\build\grenade_primer_tests.exe"
if ($LASTEXITCODE -ne 0) { throw 'Grenade primer tests failed' }
}
Copy-Item -LiteralPath (Join-Path $root 'build\copvr_bridge.dll') -Destination $destination -Force
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'openxr-loader\x64\bin\openxr_loader.dll') -Destination $destination -Force
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'openxr-loader\share\doc\openxr\LICENSE') -Destination (Join-Path $destination 'OPENXR-LOADER-LICENSE.txt') -Force
Write-Output 'VR bridge and OpenXR loader staged next to the development engine.'
