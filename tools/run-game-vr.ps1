param(
    [switch]$ImportOriginalConfig,
    [string]$OriginalConfig='C:\Program Files (x86)\Steam\steamapps\common\Stalker Call of Pripyat\_appdata_\user.ltx',
    [switch]$HighQuality,[switch]$MaxQuality,[switch]$PrepareOnly
)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$exe = Join-Path $root 'engine\bin\x64\Release\xrEngine.exe'
if (!(Test-Path -LiteralPath (Join-Path $root 'engine\bin\x64\Release\copvr_bridge.dll'))) { throw 'Build and stage the bridge with tools/build-bridge.ps1 first.' }
if (Get-Process xrEngine -ErrorAction SilentlyContinue) { throw 'Close the existing game before preparing or starting the VR build.' }
$sandbox = Join-Path $root 'game-vr'
if (!(Test-Path -LiteralPath (Join-Path $sandbox 'fsgame.ltx'))) {
    & (Join-Path $PSScriptRoot 'prepare-game-test.ps1') -SandboxName game-vr
}
$settings = Join-Path $sandbox 'appdata\user.ltx'
$marker = Join-Path $sandbox 'appdata\copvr-config-source.txt'
if ($ImportOriginalConfig -or !(Test-Path -LiteralPath $marker)) {
    if (!(Test-Path -LiteralPath $OriginalConfig)) { throw "Original configuration not found: $OriginalConfig. Pass -OriginalConfig with its path." }
    $backup = Join-Path $root ('reports\backups\graphics-' + (Get-Date -Format 'yyyyMMdd-HHmmss-fff'))
    New-Item -ItemType Directory -Force $backup | Out-Null
    if (Test-Path -LiteralPath $settings) { Copy-Item -LiteralPath $settings -Destination (Join-Path $backup 'previous-vr-user.ltx') }
    Copy-Item -LiteralPath $OriginalConfig -Destination $settings -Force
    $OriginalConfig | Set-Content -LiteralPath $marker -Encoding utf8
    Write-Host "Imported original settings: $OriginalConfig"
}
# Presets are now opt-in. Normal launches never rewrite quality, distances,
# effects, resolution, window mode or VSync. In-game changes remain saved.
if ($HighQuality -or $MaxQuality) {
    $preset = if ($MaxQuality) { 'Extreme' } else { 'High' }
    $existing = @(Get-Content -LiteralPath $settings | Where-Object { ($_ -split '\s+',2)[0] -ne '_preset' })
    $existing + @("_preset $preset") | Set-Content -LiteralPath $settings -Encoding ascii
}
# VR compatibility constraints apply after any imported/native preset.
# Keep all unrelated user quality settings intact.
$compat = [ordered]@{
    rs_fullscreen = 'off'
    vid_window_mode = 'st_opt_windowed_borderless'
    r3_msaa = 'st_opt_off'
    r3_msaa_alphatest = 'st_opt_off'
    r2_ssao_hdao = 'off'
    r2_ssao_hbao = 'on'
    r2_ssao_mode = 'hbao'
}
$lines = @(Get-Content -LiteralPath $settings | Where-Object { -not $compat.Contains(($_ -split '\s+',2)[0]) })
$lines += @($compat.GetEnumerator() | ForEach-Object { "$($_.Key) $($_.Value)" })
$lines | Set-Content -LiteralPath $settings -Encoding ascii
Write-Host "VR settings: $settings"
if ($PrepareOnly) { Write-Host 'Configuration prepared; game not launched.'; return }
Write-Host 'Starting CoP VR with saved graphics settings (DX11 renderer required).'
$process = Start-Process -FilePath $exe -WorkingDirectory $sandbox -ArgumentList '-fsltx fsgame.ltx -noprefetch -copvr -nointro -r4' -WindowStyle Hidden -PassThru
New-Item -ItemType Directory -Force (Join-Path $root 'reports') | Out-Null
$process.Id | Set-Content -LiteralPath (Join-Path $root 'reports\game-vr-pid.txt')
Write-Host "VR engine PID: $($process.Id). Logs: game-vr/appdata/logs/"
