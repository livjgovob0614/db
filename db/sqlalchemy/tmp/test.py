import sqlalchemy
from sqlalchemy import create_engine
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy import Column, Integer, String
from sqlalchemy import *


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
    Base.metadata.create_all(engine)

  def __repr__(self):
      return "<User('%s', '%s', '%s')>" % (self.name, self.fullname, self.password)


print (sqlalchemy.__version__)
engine = create_engine('sqlite:///:memory:', echo=True)
Base = declarative_base()

# 세션 만들기 - engine과 같은 레벨에서 생성 (트랜잭션을 위한 다른 인자에 대한건 이후)
Session = sessionmaker(bind=engine)
# 모듈레벨이라면: Session=sessionmaker()  \  Sssion.configure(bind=engine)

# db와 연결하려면, 새 객체 생성
session = Session()

# 매핑된 클래스로 인스턴스 만들고 새 객체 추가(실제 db에 발행된 건 아니고 pending)
ed_user = User('user1', 'User Kim', '1234')
ed_user.name
session.add(ed_user)

# 새 query 객체 생성
our_user = session.query(User).filter_by(name='user1').first()
our_user # <User('user1', 'User Kim', 'secret)>
ed_user is our_user #true

# 여러 객체 추가
session.add_add([ User('Lee', 'A Lee', 'foobar'), User('bock', 'J Bock', 'ji')])

# 비번 바꾸기
ed_user.password = 'test1234' # 당연 session에서도 바뀜

# 세션에 pending된 것들 실행시키기(반영)
ed_user.id
session.commit()
ed_user.id # 1

# 값을 변경하거나, 새로운 객체 add한 후, session.query(User).filter..하면 현재 트랜젝션 /
# 롤백 실행하면 변경 전 상태로 돌아감. session.rollback() -> fake_user in session # false

# query 보내기
for instance in session.query(User).order_by(User.id):
  print (instance.name, instance.fullname)

for row in session.query(User, User.name).all():
    print row.User, row.name

for row in session.query(User.name.label('name_label')).all():
    print row.name_label

from sqlalchemy.orm import aliased
user_alias = aliased(User, name='user_alias')
for row in session.query(user_alias, user_alias.name).all():
    print row.user_alias

# LIMIT or OFFSET
for user in session.query(User).order_by(User.id)[1:3]:
	print user

# 대부분의 메소드 호출은 새 Query 객체를 반환. chaining method 
for name in session.query(User).\
            filter(User.name=='haruair').\
            filter(User.fullname=='Edward Kim'):
    print user
