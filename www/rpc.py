from jsonrpc import handleCGI, ServiceMethod
import usermanagment
from db import Session, Problem
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

if __name__ == "__main__":
    handleCGI()
