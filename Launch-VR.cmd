@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\run-vr.ps1" -Seconds 120
pause
