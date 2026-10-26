@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\run-game-vr.ps1" -EyeWidth 2080 -EyeHeight 2096
if errorlevel 1 pause
