import PGE
import os
import uuid
import requests
import json
import os
from requests import get
import random
import time

def getserverip(uri,pathca):
      try:
        headers={"Accept":"application/json", "Content-Type":"application/json" }
        payload={}
        res=requests.get(uri, data=json.dumps(payload), headers=headers,verify=pathca)
        r= json.loads(res.text)
        if r['Status']=='Success':
           return r['IP']
        else:
           return None
      except:
          return None

def postrequest(uri,pathca,payload):
    try:
        headers={"Accept":"application/json", "Content-Type":"application/json" }
        res=requests.post(uri, data=json.dumps(payload), headers=headers,verify=pathca)
        r= json.loads(res.text)
        if r['Err_msg']=='Success':
           return 0
        else:
           return 1
    except:
           return 2

def do_upload(servuri, pathca, imgfolder,pathpubk):
  #mac=str(uuid.uuid1()).split('-')[-1]
  pubkstr=open(pathpubk).read()
  srvip=None
  pge=PGE.PGECipher(pubkstr)
  while(True):
    fl=os.listdir(imgfolder)
    n=0
    while(srvip is None and len(fl)>0):
        srvip=getserverip(servuri, pathca)
        time.sleep(random.randint(5,10)+n*60)
        n+=1
    uri='https://{}:8004/idfdv_done'.format(srvip)
    fl=os.listdir(imgfolder)
    for i in fl:
          time.sleep(5)
          p=os.path.join(imgfolder, i)
          if os.path.exists(p):
             s=open(p, 'rb').read()
             #print 'plain text len:', len(s)
             enc=pge.encrypt(s)
             #print 'enc len:',len(enc)
             sn=i.split('_')[1]
             msg={'Serial_No':sn,'data':enc, 'filename':pge.encrypt(i)}
             #ret=postrequest(uri, pathca, msg)
             ret=postrequest(uri, False,msg)
             if(ret==0):
                 os.remove(p)
             elif ret==2:
                 srvip=None
                 time.sleep(random.randint(5,10))
                 break
             else:
                 continue
    if len(fl)==0:
	time.sleep(random.randint(5,10)+300)



if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-ca', '--pathca')
    parser.add_argument('-k', '--pathpubk')
    parser.add_argument('-u', '--srvuri')
    parser.add_argument('-i', '--pathimg')
    opt = parser.parse_args()
    import sys
    reload(sys)
    do_upload(opt.srvuri, opt.pathca, opt.pathimg,opt.pathpubk)
    #idfdvcomplete.py -ca cacert.pem -k pubk.pem -u https://118.31.14.72:8002/GetServer/IDFDVUPLDSRV -i images
                 
