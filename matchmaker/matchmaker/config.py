from functools import lru_cache

from pydantic_settings import BaseSettings


class Config(BaseSettings):
    database_url: str = "sqlite:///./data.db"
    jwt_secret: str = "09d25e094faa6ca2556c818166b7a9563b93f7099f6f0f4caa6cf63b88e8d3e7"
    jwt_expire_minutes: int = 44640
    auth_password: str = "public"
    web_host: str = "127.0.0.1"
    web_port: int = 3000
    log_level: str = "info"


@lru_cache()
def get_config():
    return Config()
