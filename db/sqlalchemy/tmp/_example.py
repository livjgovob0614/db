from sqlalchemy import create_engine
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy import Column, Integer, String
from sqlalchemy.orm import sessionmaker

engine = create_engine('sqlite:///:memory:', echo=True)
Base = declarative_base()

class User(Base):
  __tablename__ = 'users'

  id = Column(Integer, primary_key=True)
  name = Column(String(50))
  fullname = Column(String(50))
  password = Column(String(12))

  def __init__(self, name, fullname, password):
    self.name = name
    self.fullname = fullname
    self.password = password

  def __repr__(self):
      return "<User('%s', '%s', '%s')>" % (self.name, self.fullname, self.password)

class Post(Base):
  __tablename__ = 'post'

  author_id = Column(Integer, primary_key=True)
  author = Column(String(50))


Base.metadata.create_all(engine)
Session = sessionmaker(bind=engine)
session = Session()


# table User
ed_user = User('user1', 'User Kim', '1234')
session.add(ed_user)
session.add_all([ User('Lee', 'A Lee', 'foobar'), User('bock', 'J Bock', 'ji')])
ed_user.password = 'test1234'

session.commit()


print ("---------------------")
# all(), [1:3] 등 해야 value 담김
rows = session.query(User).filter(User.name=='user1').order_by(User.id)[0:2]
print (rows)
print ("---------------------")
for name in session.query(User).filter(User.name=='bock'):
  print (name)

# table Post
session.add_all([Post('Lee'), Post('bock')])
session.commit()
print ("---------------------")
for i in session.query(Post).join(User).option(contains_eager(User.name)):
  print (i)


