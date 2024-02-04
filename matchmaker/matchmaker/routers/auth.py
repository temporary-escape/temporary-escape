import uuid
from datetime import datetime, timezone
from typing import Annotated

from fastapi import APIRouter, Response, Depends
from pydantic import BaseModel
from matchmaker.auth import authenticate_user, JwtAuth


router = APIRouter(prefix="/auth", tags=["Authentication"])


class LoginModel(BaseModel):
    username: uuid.UUID
    password: str


@router.post("/login", summary="Public login endpoint")
async def auth_login(body: LoginModel):
    token, expires = authenticate_user(body.username, body.password)

    response = Response(status_code=201)
    response.set_cookie(
        "Authorization", f"Bearer {token}", httponly=True, expires=expires
    )
    return response


@router.post("/logout", summary="Public logout endpoint")
async def auth_login(user: JwtAuth):
    response = Response(status_code=201)
    response.set_cookie(
        "Authorization", "", httponly=True, expires=datetime.now(tz=timezone.utc)
    )
    return response
