import uuid
from datetime import datetime, timezone, timedelta
from typing import Tuple, Annotated

from fastapi import Request, HTTPException, Depends
from jose import JWTError, jwt
from pydantic import BaseModel

from matchmaker.config import get_config

ALGORITHM = "HS256"


class User(BaseModel):
    username: uuid.UUID


def _create_jwt_token(data: dict, expire: datetime) -> str:
    config = get_config()

    to_encode = data.copy()
    to_encode.update({"exp": expire})
    encoded_jwt = jwt.encode(to_encode, config.jwt_secret, algorithm=ALGORITHM)
    return encoded_jwt


def validate_user(auth: str) -> User:
    config = get_config()

    credentials_exception = HTTPException(
        status_code=401,
        detail="Not authenticated",
    )

    bearer, token = auth.split(" ", maxsplit=1)
    if bearer.lower() != "bearer":
        raise credentials_exception

    try:
        payload = jwt.decode(token, config.jwt_secret, algorithms=[ALGORITHM])
        username: str = payload.get("sub")
        if username is None:
            raise credentials_exception

        return User(username=uuid.UUID(username))
    except JWTError:
        raise credentials_exception
    except TypeError | ValueError:
        raise credentials_exception


async def get_user(request: Request) -> User:
    credentials_exception = HTTPException(
        status_code=401,
        detail="Not authenticated",
    )

    auth = request.cookies.get("Authorization")
    if not auth:
        raise credentials_exception

    return validate_user(auth)


def authenticate_user(username: uuid.UUID, password: str) -> Tuple[str, datetime]:
    config = get_config()
    if password != config.auth_password:
        raise HTTPException(
            status_code=403,
            detail="Incorrect credentials",
        )

    expire = datetime.now(timezone.utc) + timedelta(minutes=config.jwt_expire_minutes)

    data = {
        "sub": str(username),
    }
    return _create_jwt_token(data, expire), expire


JwtAuth = Annotated[User, Depends(get_user)]
