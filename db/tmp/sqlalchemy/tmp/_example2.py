from sqlalchemy import create_engine, ForeignKey, Column, Integer, String
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker, relationship, backref

Base = declarative_base()

class User(Base):
  __tablename__ = 'users'
  __bind_key__='user'

  id = Column(Integer, primary_key=True)
  name = Column(String(50))
  fullname = Column(String(50))
  password = Column(String(12))

  def __init__(self, name, fullname, password):
    self.name = name
    self.fullname = fullname
    self.password = password

  def __repr__(self):
      return "User('%s', '%s', '%s')" % (self.name, self.fullname, self.password)


class Post(Base):
  __tablename__ = 'post'
  __bind_key__='post'

  id = Column(Integer, primary_key=True)
  author = Column(String(50))
  user_id = Column(Integer, ForeignKey('users.id'))

  user = relationship("User", backref=backref('addresses', order_by=id))

  def __init__(self, author):
    self.author = author
  
  def __repr__(self):
    return "Post('%s')" % self.author


def InsertData(session):
engine = create_engine('sqlite:///:memory:', echo=True)
Base.metadata.create_all(engine)
Session = sessionmaker(bind=engine)
session = Session()


# table User
ed_user = User('user1', 'User Kim', '1234')
session.add(ed_user)
session.add_all([ User('Lee', 'A Lee', 'foobar'), User('bock', 'J Bock', 'ji')])
ed_user.password = 'test1234'

session.commit()


# table Post
session.add_all([Post('Lee'), Post('bock')])
session.commit()

#for u, a in session.query(User, Post).filter(User.name==Post.author).all():
#  print (u, a)
name = session.query(User.name).filter(User.name==Post.author).all()
print ("---------------------")
print (name)





# select (using join) #
  #(1)
  #post = session.query(Post).filter(User.name==Post.author).all()
  #(2)
  post = session.query(Post).filter(Post.genre.like('comic%')).join(User, User.name==Post.author).all()

  ######## post_list = [n for n, in post] # [n[0] for n in num]  ##### 요건 table의 한 column만 가져올 땐 이렇게 해야하던데...

  for i in post:
    name = i.author
    num = i.number
    if (num < 100):
      result_list.append(name)
  return result_list
