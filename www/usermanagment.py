from os import urandom
from base64 import b64encode
from db import User, Session, Login
from sqlalchemy import create_engine, Table, Column, Integer, String, MetaData, ForeignKey, and_, or_, ForeignKey, Text
from datetime import datetime

def logout(cookie):
    session = Session()
    x = session.query(Login).filter_by(cookie=cookie).one()
    if x: del x
    session.commit()

def login(handle,password):
    session = Session()
    q = session.query(User).filter(and_(User.handle==handle,User.password==password))
    if q.count() == 0:
        return None
    else:
        uid = q.one().id
        i = b64encode(urandom(32))
        #Hope that it does not colide
        l = Login()
        l.uid = uid
        l.cookie = i
        l.time = datetime.utcnow()
        session.add(l)
        session.commit()
        return i

def getUid(cookie):
    session = Session()
    x = session.query(Login).filter_by(cookie=cookie).one()
    if not x:
        raise Exception("Not logged in")
    return x.uid
    

