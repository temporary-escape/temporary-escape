import asyncio
from datetime import datetime, timezone
from matchmaker.db import Database
from fastapi import APIRouter, Response
from pydantic import BaseModel
from matchmaker.auth import create_auth_state, authenticate_user, JwtAuth

router = APIRouter(prefix="/v1/auth", tags=["Authentication"])


class AuthStateCreatedResponse(BaseModel):
    state: str


class AuthMeResponse(BaseModel):
    user_id: str


@router.post(
    "/state",
    summary="Request unique state token for the OAuth flow",
    status_code=200,
    responses={
        200: {
            "description": "Success",
            "model": AuthStateCreatedResponse,
        },
    },
)
async def post_create_auth_state(db: Database) -> AuthStateCreatedResponse:
    state = create_auth_state(db)

    # Avoid DDoS
    await asyncio.sleep(1.0)

    return AuthStateCreatedResponse(
        state=state,
    )


class LogInRequest(BaseModel):
    state: str


@router.post(
    "/login",
    summary="Check if the user has completed the oauth flow",
    status_code=201,
    responses={
        201: {
            "description": "Success",
        },
        204: {
            "description": "User has not yet completed the OAuth flow",
        },
    },
)
async def post_auth_login(body: LogInRequest, db: Database):
    token = authenticate_user(db, body.state)

    if not token:
        return Response(status_code=204)

    response = Response(status_code=201)
    response.set_cookie(
        "Authorization",
        f"Bearer {token}",
        httponly=True,
    )
    return response


@router.post(
    "/logout",
    summary="Log out",
    status_code=201,
)
async def post_auth_logout():
    response = Response(status_code=201)
    response.set_cookie(
        "Authorization", "", httponly=True, expires=datetime.now(tz=timezone.utc)
    )
    return response


@router.get(
    "/me",
    summary="Check if we are logged in",
    status_code=200,
)
async def get_auth_me(user: JwtAuth) -> AuthMeResponse:
    return AuthMeResponse(
        user_id=user,
    )
