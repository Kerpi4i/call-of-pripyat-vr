$ErrorActionPreference='Stop'
Add-Type -AssemblyName System.Windows.Forms
$installRoot=[IO.Path]::GetFullPath($PSScriptRoot).TrimEnd('\')
if (!(Test-Path -LiteralPath (Join-Path $installRoot 'copvr-install.marker'))) { throw 'Installation marker missing; refusing removal.' }
if (Get-Process xrEngine,CoPVR -ErrorAction SilentlyContinue) { [Windows.Forms.MessageBox]::Show('Close the game and launcher first. / Закрийте гру та лаунчер.'); exit 1 }
if ([Windows.Forms.MessageBox]::Show('Remove CoPVR? Saves and settings in data/ will be kept. Original game files are not touched. / Видалити CoPVR? Збереження та налаштування в data/ залишаться.','CoPVR', 'YesNo') -ne 'Yes') { exit }
$manifest=Join-Path $installRoot 'installed-files.txt'
foreach($relative in Get-Content -LiteralPath $manifest) {
    $target=[IO.Path]::GetFullPath((Join-Path $installRoot $relative))
    if (!$target.StartsWith($installRoot+'',[StringComparison]::OrdinalIgnoreCase) -or $relative -match '^data[\/]') { throw 'Invalid managed path.' }
    if (Test-Path -LiteralPath $target -PathType Leaf) { Remove-Item -LiteralPath $target -Force }
}
$key='HKCU:\Software\Microsoft\Windows\CurrentVersion\Uninstall\CoPVR-0.1.0-alpha.1'
if (Test-Path -LiteralPath $key) {
    if ((Get-ItemProperty -LiteralPath $key).InstallLocation -eq $installRoot) { Remove-Item -LiteralPath $key }
}
$shortcut=Join-Path ([Environment]::GetFolderPath('DesktopDirectory')) 'Call of Pripyat VR.lnk'
if(Test-Path -LiteralPath $shortcut) {
    $shell=New-Object -ComObject WScript.Shell
    if($shell.CreateShortcut($shortcut).TargetPath -eq (Join-Path $installRoot 'CoPVR.exe')) { Remove-Item -LiteralPath $shortcut }
}
foreach($name in @('fsgame.ltx','game-path.txt','copvr-install.marker','installed-files.txt','integrity.sha256')) {
    $target=Join-Path $installRoot $name
    if(Test-Path -LiteralPath $target){Remove-Item -LiteralPath $target -Force}
}
[Windows.Forms.MessageBox]::Show('Removed. Your data/ folder was preserved. / Видалено. Папку data/ збережено.','CoPVR') | Out-Null
