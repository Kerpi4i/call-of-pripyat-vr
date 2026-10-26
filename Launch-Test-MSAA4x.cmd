@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\run-game-vr.ps1" -Msaa 4x
if errorlevel 1 pause
