@echo off
setlocal
pwsh -NoProfile -ExecutionPolicy Bypass -File "%~dp0build_all.ps1" %*
if %ERRORLEVEL% NEQ 0 (
    powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0build_all.ps1" %*
)
endlocal
