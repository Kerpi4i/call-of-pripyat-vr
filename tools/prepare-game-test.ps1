param([string]$GamePath='C:\Program Files (x86)\Steam\steamapps\common\Stalker Call of Pripyat',
      [ValidateSet('game-test','game-vr')][string]$SandboxName='game-test')
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$game = (Resolve-Path -LiteralPath $GamePath).Path.TrimEnd('\') + '\'
$sandbox = Join-Path $root $SandboxName
$appdata = Join-Path $sandbox 'appdata'
$overlay = Join-Path $sandbox 'gamedata'
New-Item -ItemType Directory -Force $sandbox,$appdata | Out-Null
$lines = foreach ($line in Get-Content -LiteralPath (Join-Path $game 'fsgame.ltx')) {
    if ($line -match '^\$app_data_root\$\s*=') { '$app_data_root$ = true|false| ' + $appdata + '\' }
    elseif ($line -match '^\$game_data\$\s*=') { '$game_data$ = false|true| ' + $overlay + '\' }
    else { $line.Replace('$fs_root$',$game) }
}
$lines | Set-Content -LiteralPath (Join-Path $sandbox 'fsgame.ltx') -Encoding ascii
Copy-Item -LiteralPath (Join-Path $root 'engine\res\gamedata') -Destination $sandbox -Recurse -Force
if(Test-Path -LiteralPath (Join-Path $root 'assets\gamedata')) {
    Copy-Item -LiteralPath (Join-Path $root 'assets\gamedata') -Destination $sandbox -Recurse -Force
}
if(Test-Path -LiteralPath (Join-Path $root 'release\gamedata')) {
    Copy-Item -LiteralPath (Join-Path $root 'release\gamedata') -Destination $sandbox -Recurse -Force
}
$bin = Join-Path $root 'engine\bin\x64\Release'
foreach ($file in Get-ChildItem -LiteralPath (Join-Path $root 'engine\sdk\binaries\x64') -File) {
    if ($file.Extension -eq '.dll') { Copy-Item -LiteralPath $file.FullName -Destination $bin -Force }
}
if (!(Test-Path -LiteralPath (Join-Path $appdata 'user.ltx'))) {
    @('renderer renderer_r4','rs_fullscreen off','vid_mode 1280x720','snd_volume_eff 0.5','snd_volume_music 0.3') |
        Set-Content -LiteralPath (Join-Path $appdata 'user.ltx') -Encoding ascii
}
Write-Output 'Flat engine test files prepared in game-test/. Game archives are referenced at their original location; settings and saves are separate.'
Write-Output "Engine: $bin\xrEngine.exe"
Write-Output "Working directory: $sandbox"
