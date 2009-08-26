import problemmanagment
import usermanagment
import tempfile
import subprocess
import os
import ConfigParser
import usermanagment
from db import *

def submit(req):
    msg=""
    try: 
    #Todo assert logged in
    #source = req.form['source']
    #x=repr(source)
        source = ""
        fn = ""
        lang = ""
        problem = -1
        cookie = ""
        if 'source' in req.form: source=req.form['source']
        if 'problem' in req.form: problem=int(req.form['problem'])
        if 'lang' in req.form: lang=req.form['lang']
        if 'cookie' in req.form: cookie=req.form['cookie']
        if 'file' in req.form: 
            z=req.form['file']
            fn=z.filename
            z = z.file.read()
            if z: source = z
        uid = usermanagment.getUid(cookie)
        msg = problemmanagment.submit(uid,problem,lang,source,fn)
    except Exception as e:
        msg = str(e)

    req.content_type = "text/html"
    if msg:
        msg = msg.replace('\\','\\\\').replace('"','\\"')
        return "<html><body><script>alert(\"%s\")</script></body></html>"%msg
    else:
        return """<html><body><script>window.parent.document.modules.main.main.switchToStatusTab();</script></body></html>"""

def problem(req):
    msg=""
    try:
        uid = usermanagment.getUid(req.form['cookie'])
        dir = tempfile.mkdtemp('djudge')
        try:
            z = req.form['file']
            
            if z.type in ['application/x-bzip','application/x-gzip','application/x-tar']:
                if z.type == 'application/x-bzip': d='j'
                elif z.type == 'application/x-gzip': d='z'
                else: d=''
                p = subprocess.Popen(["/bin/tar",'-x'+d,"-C",dir], stdin=z.file, stderr=subprocess.PIPE)
                res = p.communicate()[1]
                if p.returncode != 0:
                    raise Exception("untarning archive failed: "+res)
            else:
                msg = str(z.type)
                
            basedir=None
            for (path,dirs,files) in os.walk(dir):
                if 'config.ini' in files:
                    basedir=path
            if not basedir:
                raise Exception("config.ini was not found")

            config = ConfigParser.ConfigParser()
            config.read(os.path.join(basedir,'config.ini'))
            name = config.get('problem','name')
            desc = config.get('problem','description')
            
            session = Session()
            if session.query(Problem).filter_by(name=name).count() != 0:
                raise Exception("There is already a problem called '%s'"%name)
            
            problem = Problem(name,desc)
            problem.submitter_id = uid
            problem.hash = None
            problem.done = False
            session.add(problem)
            session.commit()
            
            pd = "/home/jakobt/problems/"+name
            ar = "/home/jakobt/problems/"+name+".tar.bz2"
            os.rename(basedir, pd)
            


            #!/bin/bash]# mkdir -p mozdoc
# cd mozdoc
# echo | htlatex ../text.tex "xhtml,uni-html4,mozilla,charset=utf-8,mathml-" " -cmozhtf -cunihtf -utf8" "-cvalidate"
# cd ..
# mkdir -p gendoc
# cd gendoc
# echo | htlatex ../text.tex "xhtml,uni-html4,charset=utf-8" " -cunihtf -utf8" "-cvalidate"
# cd ..
# mkdir -p pdfdoc
# cd pdfdoc
# echo | latex ../text.tex
# cd

            p = subprocess.Popen(["/bin/tar",'-C',pd,'-cjf',ar,'.'], stderr=subprocess.PIPE)
            res = p.communicate()[1]
            if p.returncode != 0:
                raise Exception("creating archive failed: "+res)
            
            p = subprocess.Popen(["/usr/bin/sha512sum",'-b',ar], stdout=subprocess.PIPE)
            res = p.communicate()[0]
            if p.returncode != 0:
                raise Exception("hashing failed: "+res)
            problem.hash = res.split(" ")[0]
            session.commit()
            client.importProblem(res, ar)
        finally:
            for (path,dirs,files) in os.walk(dir, topdown=False):
                for file in files:
                    os.unlink(os.path.join(path,file))
                os.rmdir(path)
        pass
    except ConfigParser.Error as e:
        msg = "Error parsing config.ini: "+str(e)
    except Exception as e:
        msg = "Exception: "+str(e)

    req.content_type = "text/html"
    if msg:
        msg = msg.replace('\\','\\\\').replace('"','\\"')
        return "<html><body><script>alert(\"%s\")</script></body></html>"%msg
    else:
        return """<html><body><script>window.parent.document.modules.main.main.switchToStatusTab();</script></body></html>"""

        
