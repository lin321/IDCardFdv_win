@echo off
setlocal enabledelayedexpansion
schtasks /delete /tn IDCardFdvUpgrade /f
set num=100
set /a HH = (%random%%%(27-6+1)+6)%%24 + num
set /a MM = %random%%%60 + num
schtasks /create /sc daily /st !HH:~-2!:!MM:~-2!:00 /tn IDCardFdvUpgrade /tr %~dp0upgrade_idfdv_run.vbs 