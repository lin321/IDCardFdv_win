import os
import requests
import json
import os
from requests import get
import time
import urllib
import hashlib
import random
import shutil

# wx:file lock
if os.name == 'nt':
    import win32con,win32file,pywintypes
    LOCK_EX = win32con.LOCKFILE_EXCLUSIVE_LOCK
    LOCK_SH = 0
    LOCK_NB = win32con.LOCKFILE_FAIL_IMMEDIATELY
    __overlapped = pywintypes.OVERLAPPED()
    def lock(file,flags):
        hfile = win32file._get_osfhandle(file.fileno())
        win32file.LockFileEx(hfile,flags,0,0xffff0000,__overlapped)
    def unlock(file):
        hfile = win32file._get_osfhandle(file.fileno())
        win32file.UnlockFileEx(hfile,0,0xffff0000,__overlapped)
elif os.name =='posix':
    from fcntl import LOCK_EX,LOCK_SH,LOCK_NB
    def lock(file,flags):
        fcntl.flock(file.fileno(),flags)
    def unlock(file):
        fcntl.flock(file.fileno(),fcntl.LOCK_UN)
else:
    raise RuntimeError("not supported")

def getserverip(srvuri,pathca):
        headers={"Accept":"application/json", "Content-Type":"application/json" }
        payload={}
        res=requests.get(srvuri, data=json.dumps(payload), headers=headers,verify=pathca)
        r= json.loads(res.text)
        r=json.loads(r)
        if r['Status']=='Success':
           return r['Vidx Lan IP address']
        else:
           return None

def getidfdvfilelist(uri,pathca,platform,domain):
    try:
        headers={"Accept":"application/json", "Content-Type":"application/json" }
        payload={"PLATFORM":platform,'DOMAIN':domain}
        res=requests.get(uri, data=json.dumps(payload), headers=headers,verify=pathca)
        r= json.loads(res.text)
        if r['Status']=='Success':
           return {"PLATFORM":r["PLATFORM"],"HASH":r["HASH"],"VERSION":r["IDFDV_VERSION"],"IP":r["IP"],"PORT":r["PORT"],"FILELIST":r["FILELIST"]}
        else:
           return None
    except:
        return None

def getidfdvfiles(IP, PORT, HASH, filelist, path):
        p=os.path.join(path, HASH)
        if not os.path.exists(p):
            os.makedirs(p)
        for i in filelist:
            try:
                 name=i['name']
                 hashval=i['SHA256']
                 uriroot="http://{}:{}/{}".format(IP,PORT,HASH)
                 url=uriroot+'/'+name
                 response = urllib.urlopen(url)
                 data=response.read()
                 if hashval.lower() !=hashlib.sha256(data).hexdigest().lower():
                      return False
                 fn=os.path.join(p,name)
                 f=open(fn, 'wb')
                 f.write(data)
                 f.close()
            except:
                 return False
        return True

import re
def compareversion(v1,v2):
    v1_=re.findall('[0-9.]+',v1)
    if len(v1_)!=0:
        v1_=v1_[-1]
    if len(v1_)==0:
        return -1
    v2_=re.findall('[0-9.]+',v2)
    if len(v2_)!=0:
        v2_=v2_[-1]
    if len(v2_)==0:
        return 1
    v1_=v1_.split('.')
    v2_=v2_.split('.')
    len1=len(v1_)
    len2=len(v2_)
    if len1>len2:
        for i in range(len1):
            if i >=len2:
                if v1_[i] > 0:
                   return 1
            elif v1_[i] > v2_[i]:
                return 1
            elif v1_[i] < v2_[i]:
                return -1
        return 0
    else:
        for i in range(len1):
            if i >=len1:
               if v2_[i] > 0:
                   return -1
            elif v1_[i] > v2_[i]:
               return 1
            elif v1_[i] < v2_[i]:
               return -1
        return 0


def do_upgrade(srvuri, pathca, platform, domain, destpath):
  n=0
  curhash=""
  curversion=""
  vp=os.path.join(destpath,'version.txt')
  if os.path.exists(vp):
      curversion=open(vp).read()
      curhash=hashlib.md5(curversion).hexdigest()
  verhash=""
  while(n<10):
       n+=1
       ret=getidfdvfilelist(srvuri, pathca,platform,domain)
       if ret is None:
          time.sleep(random.randint(5,10)+300)
          continue
       print ret
       verhash=ret['HASH'].lower()
       if verhash==curhash.lower():
          break
       idfdvversion=ret['VERSION']
       if compareversion(curversion,idfdvversion) >= 0:
           break      
       filelist=[]
       for i in ret['FILELIST']:
             name=i['name']
             hashv=i['SHA256'].lower()
             p=os.path.join(destpath,name)
             if not os.path.exists(p):
                 filelist.append(i)
             elif hashlib.sha256(open(p).read()).hexdigest().lower()!=hashv:
                 p2=os.path.join(destpath,verhash,name)
                 if not os.path.exists(p2) or hashlib.sha256(open(p2).read()).hexdigest().lower()!=hashv:
                     filelist.append(i)
                 else:
                     n=0

       if len(filelist)>0:
            r=getidfdvfiles(ret['IP'], ret['PORT'], verhash, filelist, destpath)
            if not r:
                 #time.sleep(random.randint(5,10)+300)
                 continue
       try:
           tmpp=os.path.join(destpath,verhash)
           vp=open(os.path.join(destpath,'version.txt'),"r+")
           lock(vp,LOCK_EX)
           for i in os.listdir(tmpp):
                if i != 'version.txt':
                    shutil.copy(os.path.join(tmpp,i), destpath)
           unlock(vp)
           vp.close()
           shutil.copy(os.path.join(tmpp,'version.txt'), destpath)
           #os.rmdirs(tmpp)
           shutil.rmtree(tmpp)
       except:
           break
  tmpp=os.path.join(destpath,verhash)
  if os.path.exists(tmpp):
      try:
          shutil.rmtree(tmpp)
      except:
          return


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-ca', '--pathca')
    parser.add_argument('-u', '--srvuri')
    parser.add_argument('-d', '--destpath')
    parser.add_argument('-p', '--platform')
    parser.add_argument('-m', '--domain')
    opt = parser.parse_args()
    import sys
    reload(sys)
    do_upgrade(opt.srvuri, opt.pathca, opt.platform, opt.domain, opt.destpath)
    #python upgrade_idfdv.py -ca cacert.pem -u https://118.31.14.72:8002/IdfdvVersion -d C:\\mtdt\\IDCardFdv -p Windows -m IDFDVUPGRSRV
