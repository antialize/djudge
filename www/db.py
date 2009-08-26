from sqlalchemy import create_engine, Table, Column, Integer, String, MetaData, ForeignKey, and_, or_, ForeignKey, Text, DateTime, Binary, UnicodeText, Boolean
from sqlalchemy.orm import mapper, sessionmaker, relation, backref
from sqlalchemy.ext.declarative import declarative_base

Base = declarative_base()
engine = create_engine('mysql://djudge:djudge@localhost/djudge', echo=True, pool_recycle=30)

Session = sessionmaker(bind=engine)

contest_users = Table('contest_users', Base.metadata,
                      Column('contest_id', Integer, ForeignKey('contests.id')),
                      Column('user_id', Integer, ForeignKey('users.id')))

contest_problems = Table('contest_problems', Base.metadata,
                      Column('contest_id', Integer, ForeignKey('contests.id')),
                      Column('problem_id', Integer, ForeignKey('problems.id')))

class Contest(Base):
    __tablename__ = 'contests'
    id = Column(Integer, primary_key=True)
    name = Column(String(50))
    begin = Column(DateTime())
    end = Column(DateTime())
    freeze = Column(DateTime())
    open = Column(Boolean())
    
    users = relation('User', secondary=contest_users, backref='contests')
    problems = relation('Problem', secondary=contest_problems, backref='contests')
    
class User(Base):
    __tablename__ = 'users'

    id = Column(Integer, primary_key=True)
    handle = Column(String(50))
    name = Column(String(50))
    email = Column(String(50))
    password = Column(String(64))
    admin = Column(Boolean())

    def __init__(self, handle):
        self.handle = handle
        self.admin = False

class Login(Base):
    __tablename__ = 'login'
    
    id = Column(Integer, primary_key=True)
    uid = Column(Integer, ForeignKey('users.id'))
    cookie = Column(String(50))
    time = Column(DateTime)
    user = relation(User, backref=backref('logins'), order_by=id)

class Problem(Base):
    __tablename__ = 'problems'
    
    id = Column(Integer, primary_key=True)
    name = Column(String(50))
    hash = Column(String(100))
    description = Column(Text)
    done = Column(Boolean())
    public = Column(Boolean())
    submitter_id = Column(Integer, ForeignKey('users.id'))
    submitter = relation(User, backref=backref('sumbittedProblems', order_by=id))
    
    def __init__(self, name, description):
        self.name = name
        self.description = description


class Submission(Base):
    __tablename__ = 'submissions'
    
    id = Column(Integer, primary_key=True)
    jobid = Column(Integer)
    source = Column(UnicodeText)
    submitter_id = Column(Integer, ForeignKey('users.id'))
    submitter = relation(User, backref=backref('submissions', order_by=id) )
    problem_id = Column(Integer, ForeignKey('problems.id'))
    problem = relation(Problem, backref=backref('submissions', order_by=id) )
    code = Column(Integer)
    msg = Column(UnicodeText)
    submitTime = Column(DateTime())
    judgeTime = Column(DateTime())

    def __init__(self):
        pass

Base.metadata.create_all(engine)     
