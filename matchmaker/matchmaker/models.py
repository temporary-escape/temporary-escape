from pydantic import BaseModel
from sqlalchemy import Boolean, Column, ForeignKey, Integer, String, DateTime
from matchmaker.db import Base


class Server(Base):
    __tablename__ = "servers"

    id = Column(String, primary_key=True)
    name = Column(String, index=True)
    address = Column(String)
    port = Column(Integer)
    owner = Column(String)
    last_ping = Column(DateTime(timezone=True), index=True, nullable=False)
