import base64
import hashlib
from Crypto import Random
from Crypto.PublicKey import RSA
from Crypto.Cipher import AES

def genRSAKey():
    random_generator = Random.new().read
    RSAkey = RSA.generate(8192, random_generator) 
    privatek=RSAkey.exportKey('PEM')
    pubk=RSAkey.publickey().exportKey('PEM')
    f=open('/tmp/privatek.pem','w')
    f.write(privatek)
    f.close()
    f=open('/tmp/pubk.pem','w')
    f.write(pubk)
    f.close()

class PGECipher(object):
    def __init__(self, keystr): 
        self.bs = 32
        self.rsakey = RSA.importKey(keystr)
        self.keyid = hashlib.sha256('{:x}'.format(self.rsakey.n)).digest()
        self.lenkid=len(self.keyid)
    def encrypt(self, raw):
        key=Random.new().read(int(self.rsakey.n.bit_length()/8)-1)
        keyenc=self.rsakey.encrypt(key,32)[0]
        lenstr='{:04x}'.format(len(keyenc))
        raw = self._pad(raw)
        iv = Random.new().read(AES.block_size)
        key = hashlib.sha256(key).digest()
        cipher = AES.new(key, AES.MODE_CBC, iv)        
        return base64.b64encode(lenstr+self.keyid+keyenc+iv + cipher.encrypt(raw))
    def decrypt(self, enc):
        enc = base64.b64decode(enc)
        klen=int(enc[:4],16)
        start=4
        offset=start+self.lenkid
        rsaid=enc[start:offset]
        if(rsaid != self.keyid):
            return None
        start=offset
        offset=start+klen
        keyenc=enc[start:offset]
        key=self.rsakey.decrypt(keyenc)
        key = hashlib.sha256(key).digest()
        start=offset
        offset=start+AES.block_size
        iv = enc[start:offset]
        cipher = AES.new(key, AES.MODE_CBC, iv)
        return self._unpad(cipher.decrypt(enc[offset:]))#.decode('utf-8')
    def _pad(self, s):
        return s + (self.bs - len(s) % self.bs) * chr(self.bs - len(s) % self.bs)
    @staticmethod
    def _unpad(s):
        return s[:-ord(s[len(s)-1:])]



#en=PGECipher(pbk)
#de=PGECipher(prk)
#se=en.encrypt(s)
#st=de.decrypt(se)
#s==st
