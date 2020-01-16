from sqlalchemy import create_engine, Column, Integer, String
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker

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
  genre = Column(String(50))
  number = Column(Integer)

  def __init__(self, author, genre, number):
    self.author = author
    self.genre = genre
    self.number = number
  
  def __repr__(self):
    return "Post('%s')" % self.author


def run(session):
  result_list = []

  # table User #
  session.add_all([ User('user1', 'User Kim', '1234'), User('Lee', 'A Lee', 'foobar'), User('Bock', 'J Bock', 'ji')])
  session.commit()

  # table Post #
  session.add_all([Post('Lee', 'comic-1', 186), Post('Bock', 'comic-2', 23), Post('Jang', 'comic-3', 62), Post('Kim', 'horror-1', 103)])
  session.commit()

  # select #
  # (1) post = session.query(Post).filter(Post.genre.like('comic%')).filter(User.name==Post.author).all()
  # (2)
  post = session.query(Post).filter(Post.genre.like('comic%')).join(User, User.name==Post.author).all()

  # 가져온 데이터들을 하나씩 훑으면서 state를 많이 생성하는 상황
  count = 0
  for i in post:
    num = i.number
    if (num < 5):
      count += num
    if (count < 100):
      result_list.append(name)

  return result_list 


if __name__ == "__main__":
  engine = create_engine('sqlite:///:memory:', echo=True)
  Base.metadata.create_all(engine)
  Session = sessionmaker(bind=engine)
  session = Session()

  result = run(session)

  for name in result:
    print ("**************\nresult: " + name + "\n**************")
