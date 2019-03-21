cd /d %~dp0
taskkill /im idfdvcomplete.exe /t /f
start /b idfdvcomplete.exe -ca cacert.pem -k pubk.pem -u https://118.31.14.72:8002/GetServer/IDFDVUPLDSRV -i UPLOAD