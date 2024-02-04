from pydantic import BaseModel
from sqlalchemy import Boolean, Column, ForeignKey, Integer, String, DateTime, UUID
from matchmaker.db import Base


class Server(Base):
    __tablename__ = "servers"

    id = Column(String, primary_key=True)
    name = Column(String, nullable=False)
    version = Column(String, nullable=False)
    address = Column(String, nullable=True)
    port = Column(Integer, nullable=True)
    owner = Column(String, index=True, nullable=False)
    last_ping = Column(DateTime(timezone=True), index=True, nullable=False)
