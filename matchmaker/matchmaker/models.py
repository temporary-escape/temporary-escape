from sqlalchemy import Column, ForeignKey, Integer, String, DateTime
from matchmaker.db import Base
from sqlalchemy.orm import relationship


class Server(Base):
    __tablename__ = "servers"

    id = Column(String, primary_key=True)
    name = Column(String, nullable=False)
    version = Column(String, nullable=False)
    address = Column(String, nullable=True)
    port = Column(Integer, nullable=True)
    owner = Column(String, index=True, nullable=False)
    last_ping = Column(DateTime(timezone=True), index=True, nullable=False)


class AuthStateModel(Base):
    __tablename__ = "auth_state"

    id = Column(String, primary_key=True)
    state = Column(String, nullable=False)
    user_id = Column(String, ForeignKey("user.id"), index=False, nullable=True)
    created_at = Column(DateTime(timezone=True), index=True, nullable=False)


class UserSessionModel(Base):
    __tablename__ = "user_session"

    id = Column(String, primary_key=True)
    user_id = Column(
        String, ForeignKey("user.id", ondelete="CASCADE"), index=True, nullable=False
    )


class UserModel(Base):
    __tablename__ = "user"

    id = Column(String, primary_key=True)
    login = Column(String, nullable=False)
    provider = Column(String, index=True, nullable=False)
    created_at = Column(DateTime(timezone=True), index=True, nullable=False)
    updated_at = Column(DateTime(timezone=True), index=True, nullable=False)

    sessions = relationship(
        UserSessionModel,
        foreign_keys="UserSessionModel.user_id",
        passive_deletes=True,
        backref="user",
    )
