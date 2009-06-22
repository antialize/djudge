from os import urandom
from base64 import b64encode
from db import User, Session
from sqlalchemy import create_engine, Table, Column, Integer, String, MetaData, ForeignKey, and_, or_, ForeignKey, Text

cookie2uid={}

def logout(cookie):
    global cookie2uid
    if cookie in cookie2uid:
        del cookie2uid[cookie]

def login(handle,password):
    session = Session()
    q = session.query(User).filter(and_(User.handle==handle,User.password==password))
    if q.count() == 0:
        return None
    else:
        global cookie2uid
        while True:
            i = b64encode(urandom(21))
            if not i in cookie2uid: break
        cookie2uid[i] = q.one().id
        return i

def getUid(cookie):
    global cookie2uid
    if cookie in cookie2uid: return cookie2uid[cookie]
    raise Exception("Not logged in")

