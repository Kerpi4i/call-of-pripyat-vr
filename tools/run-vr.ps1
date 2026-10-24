param([switch]$Probe,[ValidateRange(1,600)][int]$Seconds=60)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$exe = Join-Path $root 'build\vr_diagnostic.exe'
if (!(Test-Path -LiteralPath $exe)) { & (Join-Path $PSScriptRoot 'build-vr.ps1') }
New-Item -ItemType Directory -Force (Join-Path $root 'reports') | Out-Null
$log = Join-Path $root ('reports\vr-' + (Get-Date -Format 'yyyyMMdd-HHmmss') + '.log')
Write-Host 'CoP VR diagnostic: standalone tracking/stereo scene; game is not loaded.'
Write-Host "Log: $log"
$errorLog = $log + '.stderr'
$vrArgs = if ($Probe) { @('--probe') } else { @('--run',"$Seconds") }
$process = Start-Process -FilePath $exe -ArgumentList $vrArgs -WindowStyle Hidden -PassThru -Wait -RedirectStandardOutput $log -RedirectStandardError $errorLog
$result = $process.ExitCode
Get-Content -LiteralPath $log | Select-String 'Runtime:|System:|Eye |Session state:|Interaction |Submitted stereo|Tracked frames:|PROBE_OK|ERROR:|HEADSET_UNAVAILABLE'
if ($result -ne 0) { Get-Content -LiteralPath $errorLog -Tail 12; Write-Host "VR diagnostic exit code: $result"; exit $result }
