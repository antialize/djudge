from jsonrpc import handleCGI, ServiceMethod
import usermanagment
import problemmanagment
import codecs
from datetime import datetime, timedelta
from db import Session, Problem, Submission, or_
@ServiceMethod
def login(handle, password):
    return usermanagment.login(handle, password)

@ServiceMethod
def logout(cookie):
    pass

@ServiceMethod
def listProblems():
    session = Session()
    return [ (x.name, x.id, x.submitter_id, x.submitter.name) for x in session.query(Problem)];


def status(code):
    if code == 0:
        return 'Accepted'
    elif code == -1:
        return 'In queue'
    else:
        return 'Reject'
        
@ServiceMethod
def listSubmissions(time=None):
    if not time:
        time = datetime.utcnow() - timedelta(hours=5)
    session = Session()
    problemmanagment.pullresults()
    return [ (x.id, x.problem.name, x.problem_id, x.submitter.handle, x.submitter_id,  x.submitter.name, status(x.code) ) for x in session.query(Submission).filter(or_(Submission.judgeTime>time,Submission.submitTime>time)) ]

if __name__ == "__main__":
    handleCGI()
