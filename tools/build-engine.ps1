param([ValidateSet('xrCore','XR_3DA','xrGame','xrRender_R4','Game')][string]$Target='Game')
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$vs = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (!$vs) { throw 'MSVC build tools are required for OpenXRay.' }
$msbuild = Join-Path $vs 'MSBuild\Current\Bin\MSBuild.exe'
$toolset = if (Test-Path -LiteralPath (Join-Path $vs 'VC\Auxiliary\Build\Microsoft.VCToolsVersion.v143.default.txt')) { 'v143' } else { 'v142' }
$targets = if ($Target -eq 'Game') { 'XR_3DA;xrGame;xrRender_R4' } else { $Target }
New-Item -ItemType Directory -Force (Join-Path $root 'reports') | Out-Null
$log = Join-Path $root 'reports\engine-build.log'
& $msbuild (Join-Path $root 'engine\src\engine.sln') "/t:$targets" /m:2 /p:Configuration=Release /p:Platform=x64 "/p:PlatformToolset=$toolset" /nologo /verbosity:minimal > $log 2>&1
$result = $LASTEXITCODE
Get-Content -LiteralPath $log -Tail 24
if ($result -ne 0) { throw "Engine build failed (exit $result). Full log: $log" }
Write-Output 'Engine binaries are in engine/bin/x64/Release. Nothing installed in the Steam game directory.'
