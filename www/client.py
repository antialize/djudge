import socket
import codecs

class PackageSocket:
    def __init__(self,s):
        self.s = s
        self.eot = False
        self.data = ""

    def read(self):
        while not '\n' in self.data:
            r = self.s.recv(1024*128)
            if not r: 
                self.eot = True
                return
            self.data += r
        msg,self.data = self.data.split('\n',1)
        msg = msg.replace('\\n','\n').replace('\\\\','\\')
        return codecs.lookup('utf-8').decode(msg,'replace')[0]
    
    def write(self, msg, end_of_package=True):
        msg = codecs.lookup('utf-8').encode(msg,'replace')[0]
        msg = msg.replace('\\','\\\\').replace('\n','\\n')
        if end_of_package: msg += '\n'
        self.s.sendall(msg)

    def canRead(self): 
        return not self.eot


def getConnection():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    host = socket.gethostbyname("127.0.0.1")
    port = 13049
    s.connect( (host,port) )
    ss = PackageSocket(s)
    ss.write("asyncclient")
    r = ss.read()
    if r != "success": return
    ss.write("webcookie")
    return ss

def judge(problem, lang, source):
    s = getConnection()
    s.write("judge")
    s.write("add") #TODO translate to entry name
    s.write(lang)
    s.write(source)
    code = s.read()
    msg = s.read()
    return (code,msg)

def pullresults():
    s = getConnection()
    s.write("pullresults")
    r = []
    while True:
        jobid = s.read()
        if jobid == "": break
        code = s.read()
        msg = s.read()
        r.append( (jobid,code,msg) )
    return r
