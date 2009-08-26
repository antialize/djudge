from jsonrpc import handleCGI, ServiceMethod
import usermanagment
import problemmanagment
import codecs
from datetime import datetime, timedelta
from db import Session, Problem, Submission, or_, User
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

@ServiceMethod
def validateCookie(cookie):
    try:
        u = usermanagment.getUser(cookie)
        return [cookie, u.id, u.admin]
    except:
        return 'invalid_cookie'
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
    problemmanagment.pull()
    return [ (x.id, x.problem.name, x.problem_id, x.submitter.handle, x.submitter_id, status(x.code), str(x.judgeTime) , str(x.submitTime) ) for x in session.query(Submission).filter(or_(Submission.judgeTime>time,Submission.submitTime>time)).order_by(Submission.submitTime) ]

@ServiceMethod
def listUsers():
    session = Session()
    return [ (x.id, x.handle, x.name) for x in session.query(User)];


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

@ServiceMethod
def getUserDetails(cookie, uid):
    session = Session()
    s = session.query(User).filter_by(id=uid).one()
    
    if not s: return None
    return {'id': s.id,
            'handle': s.handle,
            'name': s.name,
            'email': s.email,
            'admin': s.admin}

@ServiceMethod
def updateUser(cookie, uid, handle, name, pwd, admin, email):
    session = Session()
    u = usermanagment.getUser(cookie)
    if not u.admin and uid != u.id:
        raise Exception("Invalid access %s %s"%(uid,u.id))
    if uid == -1:
        cu = User(handle)
        session.add(cu)
    else:
        q = session.query(User).filter_by(id=uid)
        if q.count() == 0: raise Exception("User not found")
        cu = q.one()
    cu.name = name
    cu.email = email
    if pwd: cu.password=pwd
    if u.admin: cu.admin = admin
    session.commit()
    return cu.id

@ServiceMethod
def deleteUser(cookie, uid):
    session = Session()
    u = usermanagment.getUser(cookie)
    if not u.admin:
        raise Exception("Invalid access %s %s"%(uid,u.id))
    
    q = session.query(User).filter_by(id=uid)
    if q.count() == 0: raise Exception("User not found")
    cu = q.one()
    session.delete(cu)
    session.commit()

    
if __name__ == "__main__":
    handleCGI()
