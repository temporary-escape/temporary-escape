import logging
from datetime import datetime, timezone, timedelta
import re
from typing import Annotated, Callable, Optional
import os.path
from fastapi import Request, Response
from fastapi.responses import HTMLResponse
from fastapi import APIRouter, Form
from fastapi.routing import APIRoute
from fastapi.responses import RedirectResponse
from urllib.parse import urlencode
from fastapi.templating import Jinja2Templates
from matchmaker.db import Database
from matchmaker.config import Config, get_config
from matchmaker.auth import (
    create_auth_token,
    oauth_login_itchio,
    validate_user,
    oauth_login_github,
    process_oauth_state_by_id,
)
from matchmaker.models import AuthStateModel, UserModel

TEMPLATE_FILES_DIR = os.path.join(os.path.dirname(__file__), "..", "..", "templates")


templates = Jinja2Templates(directory=TEMPLATE_FILES_DIR)


class UIRoute(APIRoute):
    def __init__(self, path: str, endpoint: Callable, **kwargs):
        kwargs["include_in_schema"] = False
        super().__init__(path, endpoint, **kwargs)


logger = logging.getLogger(__name__)
router = APIRouter(route_class=UIRoute)


def get_itchio_url(config: Config, state: Optional[str]) -> str:
    params = {
        "client_id": config.oauth_itchio_client_id,
        "scope": "profile",
        "response_type": "token",
        "redirect_uri": config.ouath_itchio_redirect_url,
    }
    if state:
        params["state"] = state
    return f"{config.oauth_itchio_url}?{urlencode(params)}"


def get_github_url(config: Config, state: Optional[str]) -> str:
    params = {
        "client_id": config.oauth_github_client_id,
    }
    if state:
        params["state"] = state
    return f"{config.oauth_github_url}?{urlencode(params)}"


def render_login_error(request: Request):
    return templates.TemplateResponse(
        request=request,
        name="message.html",
        context={
            "title": "Error while logging in!",
            "message": "The state token is invalid. Please try again.",
            "color": "danger",
        },
    )


def render_login_success(request: Request):
    return templates.TemplateResponse(
        request=request,
        name="message.html",
        context={
            "title": "You are logged in!",
            "message": "You can close this page and go back to the game.",
            "color": "success",
        },
    )


def fill_auth_token(response: Response, user: UserModel) -> Response:
    token = create_auth_token(user)
    response.set_cookie(
        "Authorization",
        f"Bearer {token}",
        httponly=True,
    )
    response.set_cookie(
        "AuthToken", "", httponly=True, expires=datetime.now(tz=timezone.utc)
    )
    return response


@router.get("/", response_class=HTMLResponse)
def get_ui_home(request: Request):
    return templates.TemplateResponse(
        request=request,
        name="home.html",
        context={},
    )


@router.get("/login", response_class=HTMLResponse)
def get_ui_login(request: Request, db: Database):
    config = get_config()

    token_id = request.cookies.get("AuthToken")

    state: Optional[str] = None
    if token_id:
        found = db.query(AuthStateModel).get(token_id)
        if found:
            state = found.state

    return templates.TemplateResponse(
        request=request,
        name="login.html",
        context={
            "success": False,
            "oauth_itchio_url": get_itchio_url(config, state),
            "oauth_github_url": get_github_url(config, state),
        },
    )


@router.get("/token/{state_id}", response_class=HTMLResponse)
def get_ui_token_by_id(request: Request, state_id: str, db: Database):
    # Do we have the state?
    state = db.query(AuthStateModel).get(state_id)
    if not state:
        return templates.TemplateResponse(
            request=request,
            name="error.html",
            context={
                "title": "Wrong login token!",
                "message": "You have pasted a wrong login token. Please try again.",
            },
        )

    response = None

    # Are we already logged in?
    auth_token = request.cookies.get("Authorization")
    if auth_token:
        user_id = validate_user(auth_token)
        if user_id:
            logger.info(f"Processing user_id: {user_id} state: {state_id}")
            process_oauth_state_by_id(db, user_id, state_id)
            response = render_login_success(request)

    # Not logged in, do redirect
    if not response:
        response = RedirectResponse(url="/login")

    response.set_cookie(
        "AuthToken",
        f"{state.id}",
        httponly=True,
        expires=datetime.now(timezone.utc) + timedelta(minutes=60),
    )
    return response


@router.get("/oauth/itchio/callback", response_class=HTMLResponse)
def get_oauth_itchio_callback(request: Request):
    return templates.TemplateResponse(
        request=request,
        name="oauth_itchio_callback.html",
        context={},
    )


@router.post("/oauth/itchio", response_class=HTMLResponse)
async def post_oauth_itchio(
    request: Request,
    access_token: Annotated[str, Form()],
    state: Annotated[str, Form()],
    db: Database,
):
    config = get_config()

    # Do we have the state?
    if state == "null":
        state = None
    if state:
        if not re.match("^[0-9a-z]{16}$", state):
            return render_login_error(request)
    else:
        state = None

    # Perform the OAuth loging
    user = await oauth_login_itchio(config, db, access_token, state)
    logger.info(f"Successfully logged in player: {user.id}")

    # Success
    response = render_login_success(request)
    return fill_auth_token(response, user)


@router.get("/oauth/github", response_class=HTMLResponse)
async def post_oauth_github(
    request: Request, db: Database, code: str, state: Optional[str] = None
):
    config = get_config()

    if state and not re.match("^[0-9a-z]{16}$", state):
        return render_login_error(request)

    # Perform the OAuth loging
    user = await oauth_login_github(config, db, code, state)
    logger.info(f"Successfully logged in player: {user.id}")

    # Success
    response = render_login_success(request)
    return fill_auth_token(response, user)
