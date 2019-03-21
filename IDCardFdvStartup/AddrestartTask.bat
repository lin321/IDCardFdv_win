@echo off
set OffsetSeconds=60
set PSCMD=powershell -c "Get-Date (Get-Date).AddSeconds(%OffsetSeconds%) -uformat ""%%Y-%%m-%%d %%H:%%M:%%S"""
for /f "tokens=1-2" %%i in ('%PSCMD%') do (
    set "DstDate=%%i"
    set "DstTime=%%j"
)
schtasks /delete /tn IDCardFdvRestart /f
schtasks /create /sc once /sd %DstDate% /st %DstTime% /tn IDCardFdvRestart /tr %~dp0IDCardFdvStartup.exe