@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\flash.ps1" -RestoreArcadeV1
pause
