function getfolder() 
getfolder=left(wscript.scriptfullname,instrrev(wscript.scriptfullname,"\")-1) 
end function 

path = getfolder() & "\restart.bat"
Set ws=CreateObject("WScript.Shell")
ws.Run path,0