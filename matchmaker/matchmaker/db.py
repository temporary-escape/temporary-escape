from typing import Annotated, ContextManager
from contextlib import contextmanager
from fastapi import Depends
from sqlalchemy import create_engine
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker, Session

from matchmaker.config import get_config

config = get_config()

engine = create_engine(config.database_url, connect_args={"check_same_thread": False})
SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)

Base = declarative_base()


def _get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()


@contextmanager
def get_db() -> ContextManager[Session]:
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()


Database = Annotated[Session, Depends(_get_db)]
