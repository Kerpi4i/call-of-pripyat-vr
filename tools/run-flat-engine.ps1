$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$exe = Join-Path $root 'engine\bin\x64\Release\xrEngine.exe'
$sandbox = Join-Path $root 'game-test'
if (!(Test-Path -LiteralPath $exe)) { throw 'Build the baseline engine with tools/build-engine.ps1 first.' }
if (!(Test-Path -LiteralPath (Join-Path $sandbox 'fsgame.ltx'))) { & (Join-Path $PSScriptRoot 'prepare-game-test.ps1') }
Write-Host 'Starting baseline OpenXRay in desktop mode. VR is not integrated into this build.'
Write-Host 'Settings, saves and logs: game-test/appdata/'
# Visible window is intentional: this is an interactive game launch.
Start-Process -FilePath $exe -WorkingDirectory $sandbox -ArgumentList '-fsltx fsgame.ltx -noprefetch' | Out-Null
