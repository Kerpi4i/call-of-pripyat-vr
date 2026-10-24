$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$vs = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (!$vs) { throw 'Install Visual Studio C++ desktop build tools.' }
$vcvars = Join-Path $vs 'VC\Auxiliary\Build\vcvars64.bat'
$build = Join-Path $root 'build'
New-Item -ItemType Directory -Force $build | Out-Null
# Fixed build commands; paths are quoted and no filesystem operations go through cmd.
$batch = @"
@echo off
call "$vcvars" >nul
if errorlevel 1 exit /b 1
cl /nologo /EHsc /std:c++17 /W4 /I"$root\src" "$root\tests\interaction_tests.cpp" /Fe:"$build\interaction_tests.exe" /Fo:"$build\interaction_tests.obj"
if errorlevel 1 exit /b 1
cl /nologo /EHsc /std:c++17 /W4 /wd4324 /I"$root\src" /I"$root\third_party\openxr\include" /c "$root\src\openxr_input.cpp" /Fo:"$build\openxr_input.obj"
if errorlevel 1 exit /b 1
lib /nologo /OUT:"$build\copvr_input.lib" "$build\openxr_input.obj"
exit /b %errorlevel%
"@
Set-Content -LiteralPath (Join-Path $build 'compile.cmd') -Value $batch -Encoding ascii
Push-Location $build
try {
    & $env:ComSpec /d /c (Join-Path $build 'compile.cmd')
    if ($LASTEXITCODE -ne 0) { throw 'C++ build failed.' }
    & (Join-Path $build 'interaction_tests.exe')
    if ($LASTEXITCODE -ne 0) { throw 'Interaction tests failed.' }
} finally { Pop-Location }
