from os import urandom
from base64 import b64encode
from db import User, Session, Login
from sqlalchemy import create_engine, Table, Column, Integer, String, MetaData, ForeignKey, and_, or_, ForeignKey, Text
from datetime import datetime

cookie2uid={}
session = Session()
for login in session.query(Login):
    cookie2uid[login.cookie] = login.uid
del session

def logout(cookie):
    global cookie2uid
    if cookie in cookie2uid:
        session = Session()
        x = session.query(Login).filter_by(cookie=cookie).one()
        if x: del x
        del cookie2uid[cookie]
        session.commit()

def login(handle,password):
    session = Session()
    q = session.query(User).filter(and_(User.handle==handle,User.password==password))
    if q.count() == 0:
        return None
    else:
        uid = q.one().id
        global cookie2uid
        while True:
            i = b64encode(urandom(21))
            if not i in cookie2uid: break
        
        l = Login()
        l.uid = uid
        l.cookie = i
        l.time = datetime.utcnow()
        session.add(l)
        session.commit()

        cookie2uid[i] = uid
        return i

def getUid(cookie):
    global cookie2uid
    if cookie in cookie2uid: return cookie2uid[cookie]
    raise Exception("Not logged in")

