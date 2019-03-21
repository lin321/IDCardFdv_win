function getfolder() 
getfolder=left(wscript.scriptfullname,instrrev(wscript.scriptfullname,"\")-1) 
end function 

path = getfolder() & "\addcacert.bat"
Set ws=CreateObject("WScript.Shell")
ws.Run path,0