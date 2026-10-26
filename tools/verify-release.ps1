param([string]$GamePath='C:\Program Files (x86)\Steam\steamapps\common\Stalker Call of Pripyat')
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$setup=Join-Path $root 'dist\CoPVR-0.1.0-alpha.2-Setup.exe'
$smoke=Join-Path $root ('build\install-final-check-'+(Get-Date -Format 'yyyyMMdd-HHmmssfff'))
$original=@{}
foreach($file in @('fsgame.ltx','_appdata_\user.ltx')) {if(Test-Path -LiteralPath (Join-Path $GamePath $file)){$original[$file]=(Get-FileHash -LiteralPath (Join-Path $GamePath $file)).Hash}}
$process=Start-Process -FilePath $setup -ArgumentList @('--verify-extract',('"'+$smoke+'"'),('"'+$GamePath+'"')) -WindowStyle Hidden -PassThru -Wait
if($process.ExitCode-ne 0){if(Test-Path -LiteralPath ($smoke+'.error.txt')){Get-Content -LiteralPath ($smoke+'.error.txt')};throw 'Extraction failed'}
$launcher=Start-Process -FilePath (Join-Path $smoke 'CoPVR.exe') -ArgumentList '--self-check' -WindowStyle Hidden -PassThru -Wait
if($launcher.ExitCode-ne 0){throw 'Launcher completeness check failed'}
foreach($file in $original.Keys){if((Get-FileHash -LiteralPath (Join-Path $GamePath $file)).Hash-ne $original[$file]){throw 'Original config changed'}}
# Exercise rejection of a nonempty target; no changes should occur.
$before=(Get-FileHash -LiteralPath (Join-Path $smoke 'CoPVR.exe')).Hash
$again=Start-Process -FilePath $setup -ArgumentList @('--verify-extract',('"'+$smoke+'"'),('"'+$GamePath+'"')) -WindowStyle Hidden -PassThru -Wait
if($again.ExitCode-eq 0 -or (Get-FileHash -LiteralPath (Join-Path $smoke 'CoPVR.exe')).Hash-ne $before){throw 'Existing install protection failed'}
$record=[ordered]@{date=(Get-Date -Format o);version='0.1.0-alpha.2';installer_exit=$process.ExitCode;launcher_self_check_exit=$launcher.ExitCode;nonempty_target_rejected=$true;original_configs_unchanged=$true;game_started=$false;interactive_install='pending';headset_acceptance='pending'}
$record|ConvertTo-Json|Set-Content -LiteralPath (Join-Path $root 'reports\release-final-install-check.json') -Encoding utf8
$record|ConvertTo-Json
