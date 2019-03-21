cd /d %~dp0
taskkill /im upgrade_idfdv.exe /t /f
start /b upgrade_idfdv.exe -ca cacert.pem -u https://118.31.14.72:8002/IdfdvVersion -d %~dp0..\\Upgrade -p Windows -m IDFDVUPGRSRV