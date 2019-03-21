@echo off
cd /d %~dp0
copy IDCardFdvStartup.exe ..
cd ..
start /b IDCardFdvStartup.exe