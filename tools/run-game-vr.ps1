param(
    [switch]$ImportOriginalConfig,
    [string]$OriginalConfig='C:\Program Files (x86)\Steam\steamapps\common\Stalker Call of Pripyat\_appdata_\user.ltx',
    [switch]$HighQuality,[switch]$MaxQuality,[switch]$PrepareOnly,
    [ValidateScript({$_ -eq 0 -or ($_ -ge 512 -and $_ -le 4096)})][int]$EyeWidth=2080,
    [ValidateScript({$_ -eq 0 -or ($_ -ge 512 -and $_ -le 4096)})][int]$EyeHeight=2096,
    [switch]$ConservativeShadows,
    [ValidateSet('Saved','Off','2x','4x')][string]$Msaa='Saved',
    [switch]$Profile,
    [switch]$ShadowCache,
    [switch]$NoDynamicShadows,
    [switch]$NoLocalShadows,
    [switch]$NoSunShadows,
    [switch]$ShadowTiles,
    [switch]$ShadowPipeline,
    [switch]$ShadowFaceCull,
    [switch]$SkipBaseCameraWork,
    [switch]$BinocularShadows
)
$ErrorActionPreference = 'Stop'
if ($ShadowTiles) { $ShadowCache = $true }
if (($EyeWidth -eq 0) -xor ($EyeHeight -eq 0)) { throw 'Specify both EyeWidth and EyeHeight, or leave both zero for the saved vid_mode.' }
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
    r2_ssao_hdao = 'off'
    r2_ssao_hbao = 'on'
    r2_ssao_mode = 'hbao'
}
$lines = @(Get-Content -LiteralPath $settings | Where-Object { -not $compat.Contains(($_ -split '\s+',2)[0]) })
$lines += @($compat.GetEnumerator() | ForEach-Object { "$($_.Key) $($_.Value)" })
$lines | Set-Content -LiteralPath $settings -Encoding ascii
Write-Host "VR settings: $settings"
if ($EyeWidth -gt 0) { Write-Host "Experimental eye buffers: $($EyeWidth)x$($EyeHeight) per eye; desktop window follows vid_mode." }
else { Write-Host 'Eye buffers follow the saved vid_mode (previous rendering mode).' }
if ($ConservativeShadows) { Write-Host 'Experimental conservative sun shadows enabled (TSM/focus bypassed).' }
Write-Host "MSAA: $Msaa (per-launch override; saved configuration is preserved)."
if ($Profile) { Write-Host 'CPU/GPU profiling enabled; CSV files will be written under reports/performance.' }
if ($ShadowCache) { Write-Host 'Experimental same-frame local shadow atlas cache enabled (up to 128 MiB).' }
if ($NoDynamicShadows) { Write-Host 'Diagnostic mode: dynamic sun/local caster shadows disabled; lighting retained.' }
if ($NoLocalShadows) { Write-Host 'Diagnostic mode: local caster shadows disabled; sun shadows retained unless separately disabled.' }
if ($NoSunShadows) { Write-Host 'Diagnostic mode: sun caster shadows disabled; local shadows retained unless separately disabled.' }
if ($PrepareOnly) { Write-Host 'Configuration prepared; game not launched.'; return }
Write-Host 'Starting CoP VR with saved graphics settings (DX11 renderer required).'
$arguments = '-fsltx fsgame.ltx -noprefetch -copvr -nointro -r4'
if ($EyeWidth -gt 0) { $arguments += " -copvr_eye $($EyeWidth)x$($EyeHeight)" }
if ($ConservativeShadows) { $arguments += ' -copvr_shadow_safe' }
if ($ShadowCache) { $arguments += ' -copvr_shadow_cache' }
if ($ShadowTiles) { $arguments += ' -copvr_shadow_tiles' }
if ($ShadowPipeline) { $arguments += ' -copvr_shadow_pipeline' }
if ($ShadowFaceCull) { $arguments += ' -copvr_shadow_face_cull' }
if ($SkipBaseCameraWork) { $arguments += ' -copvr_skip_base_camera_work' }
if ($BinocularShadows) { $arguments += ' -copvr_binocular_shadows' }
if ($NoDynamicShadows) { $arguments += ' -copvr_no_dynamic_shadows' }
if ($NoLocalShadows) { $arguments += ' -copvr_no_local_shadows' }
if ($NoSunShadows) { $arguments += ' -copvr_no_sun_shadows' }
if ($Msaa -ne 'Saved') {
    $samples = @{Off=0; '2x'=2; '4x'=4}[$Msaa]
    $arguments += " -copvr_msaa $samples"
}
if ($Profile) {
    $arguments += ' -copvr_profile'
    $profileDir = Join-Path $root ('reports\performance\' + (Get-Date -Format 'yyyyMMdd-HHmmss-fff'))
    New-Item -ItemType Directory -Force $profileDir | Out-Null
    [ordered]@{
        arguments = $arguments
        eyeWidth = $EyeWidth
        eyeHeight = $EyeHeight
        msaaOverride = $Msaa
        executable = $exe
        settings = $settings
    } | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $profileDir 'capture.json') -Encoding utf8
    Copy-Item -LiteralPath $settings -Destination (Join-Path $profileDir 'user.ltx')
    Write-Host "Profile output: $profileDir"
}
$previousProfileDir = $env:COPVR_PROFILE_DIR
try {
    if ($Profile) { $env:COPVR_PROFILE_DIR = $profileDir }
    $process = Start-Process -FilePath $exe -WorkingDirectory $sandbox -ArgumentList $arguments -WindowStyle Hidden -PassThru
} finally { $env:COPVR_PROFILE_DIR = $previousProfileDir }
New-Item -ItemType Directory -Force (Join-Path $root 'reports') | Out-Null
$process.Id | Set-Content -LiteralPath (Join-Path $root 'reports\game-vr-pid.txt')
Write-Host "VR engine PID: $($process.Id). Logs: game-vr/appdata/logs/"
