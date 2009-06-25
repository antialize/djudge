import re
import client
from db import Submission, Session
from datetime import datetime

def submit(uid, problem, lang, source, fn):
    #Validate parameters
    if not lang in ['cc','python','java']:
        if fn[-3:] == '.py': 
            lang = 'python'
        elif fn[-3:] in ['.cc','.c','.cpp','.cxx']:
            lang = 'cc'
        elif fn[-3:] in ['.java']:
            lang = 'java'
        else:
            p = re.compile(r'\$\$[ ]*language:[ ]*(python|cc|java)[ ]*\$\$')
            m = p.search(source)
            if m: 
                lang = m.group(1)
            else:
                #Todo detect by magic regexps
                return "Unable to detect language"
    if problem == -1:
        p = re.compile(r'\$\$[ ]*problem:[ ]*([0-9]+)[ ]*\$\$')
        m = p.search(source)
        if m:
            problem = int(m.group(1))
        else:
            p = re.compile(r'(0-9)+')
            m = p.search(fn)
            if m:
                problem = int(m.group(1))
            else:
                return "Unable to detect problem number"
    if len(source) < 5:
        return "No source code submitted"
    #Send job to overlord
    try:
        code, msg = client.judge(problem, lang, source)

        session = Session()
        
        s = Submission()
        s.submitter_id = uid
        s.source = source
        s.problem_id = problem
        s.submitTime = datetime.utcnow()
        if int(code) == 12:
            s.jobid = int(msg)
            s.code = -1
            s.msg = ''
        else:
            s.msg = msg
            s.code = code
            s.jobid = None
            s.judgeTime = datetime.utcnow()

        session.add(s)
        session.commit()
        
        return ""
    except Exception as e:
        return str(e)

def pullresults():
    session = Session()
    for (jobid,code,msg) in client.pullresults():
        submission = session.query(Submission).filter_by(jobid=jobid,code=-1).one()
        if submission:
            submission.code = code
            submission.judgeTime = datetime.utcnow()
            submission.msg = msg
    session.commit()

    
        
        

    
