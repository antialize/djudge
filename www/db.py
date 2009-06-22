from sqlalchemy import create_engine, Table, Column, Integer, String, MetaData, ForeignKey, and_, or_, ForeignKey, Text
from sqlalchemy.orm import mapper, sessionmaker, relation, backref
from sqlalchemy.ext.declarative import declarative_base

Base = declarative_base()
engine = create_engine('mysql://djudge:djudge@localhost/djudge', echo=True)
cacheengien = create_engine('sqlite:///:memory:', echo=True)

Session = sessionmaker(bind=engine)

class User(Base):
    __tablename__ = 'users'

    id = Column(Integer, primary_key=True)
    handle = Column(String(50))
    name = Column(String(50))
    password = Column(String(50))

    def __init__(self, handle, name, password):
        self.handle = handle
        self.name = name
        self.password = password

class Problem(Base):
    __tablename__ = 'problems'
    
    id = Column(Integer, primary_key=True)
    name = Column(String(50))
    description = Column(Text)
    submitter_id = Column(Integer, ForeignKey('users.id'))
    submitter = relation(User, backref=backref('sumbittedProblems', order_by=id))
    
    def __init__(self, name, description):
        self.name = name
        self.description = description


Base.metadata.create_all(engine)     
