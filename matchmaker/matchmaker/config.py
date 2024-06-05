from functools import lru_cache

from pydantic_settings import BaseSettings


class Config(BaseSettings):
    database_url: str = "sqlite:///./data.db"
    jwt_secret: str = "09d25e094faa6ca2556c818166b7a9563b93f7099f6f0f4caa6cf63b88e8d3e7"
    jwt_expire_minutes: int = 44640
    auth_password: str = "public"
    web_host: str = "127.0.0.1"
    web_port: int = 8000
    log_level: str = "info"

    # itch.io OAuth
    itchio_base_url: str = "https://itch.io"
    oauth_itchio_url: str = "https://itch.io/user/oauth"
    oauth_itchio_client_id: str = "529fae349db834897d500dd7752b8922"
    oauth_itchio_client_secret: str = ""
    ouath_itchio_redirect_url: str = (
        "https://server.temporaryescape.org/oauth/itchio/callback"
    )

    # GitHub OAuth
    github_base_url: str = "https://github.com"
    github_api_url: str = "https://api.github.com"
    oauth_github_url: str = "https://github.com/login/oauth/authorize"
    oauth_github_client_id: str = "Ov23liJLWYOo13deFAcQ"
    oauth_github_client_secret: str = ""


@lru_cache()
def get_config():
    return Config()
