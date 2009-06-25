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
    return usermanagment.logout(cookie)


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
    else:
        time = datetime.strptime(time,"%Y-%m-%d %H:%M:%S")
    
    session = Session()
    problemmanagment.pullresults()
    return [ (x.id, x.problem.name, x.problem_id, x.submitter.handle, x.submitter_id, status(x.code), str(x.judgeTime) , str(x.submitTime) ) for x in session.query(Submission).filter(or_(Submission.judgeTime>time,Submission.submitTime>time)).order_by(Submission.submitTime) ]


@ServiceMethod
def getSubmissionDetails(cookie, sid):
    session = Session()
    uid = usermanagment.getUid(cookie)
    s = session.query(Submission).filter_by(submitter_id=uid, id=sid).one()
    if not s: return None
    return {'id': s.id,
            'code': s.code,
            'message': s.msg,
            'submitTime': str(s.submitTime),
            'judgeTime': str(s.judgeTime),
            'problem_id': s.problem_id,
            'problem': s.problem.name,
            'source': s.source}
    
if __name__ == "__main__":
    handleCGI()
