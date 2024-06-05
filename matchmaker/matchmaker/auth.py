import random
import string
from datetime import datetime, timezone
from typing import Annotated

from fastapi import Request, HTTPException, Depends
from jose import JWTError, jwt
import logging
from typing import Optional
from matchmaker.db import Database
from matchmaker.config import Config, get_config
from matchmaker.internal.github_client import GitHubClient, GitHubOAuthClient
from matchmaker.models import AuthStateModel, UserModel
from matchmaker.internal.itchio_client import ItchIOClient

ALGORITHM = "HS256"


logger = logging.getLogger(__name__)


def _create_jwt_token(data: dict) -> str:
    config = get_config()

    to_encode = data.copy()
    encoded_jwt = jwt.encode(to_encode, config.jwt_secret, algorithm=ALGORITHM)
    return encoded_jwt


def validate_user(auth: str) -> Optional[str]:
    config = get_config()

    bearer, token = auth.split(" ", maxsplit=1)
    if bearer.lower() != "bearer":
        return None

    try:
        payload = jwt.decode(token, config.jwt_secret, algorithms=[ALGORITHM])
        user_id: str = payload.get("sub")
        if user_id is None:
            return None

        return user_id
    except JWTError:
        return None
    except TypeError | ValueError:
        return None


async def get_user(request: Request) -> str:
    credentials_exception = HTTPException(
        status_code=401,
        detail="Not authenticated",
    )

    auth = request.cookies.get("Authorization")
    if not auth:
        raise credentials_exception

    user_id = validate_user(auth)
    if not user_id:
        raise credentials_exception

    return user_id


def create_auth_token(user: UserModel) -> str:
    data = {
        "sub": user.id,
    }
    token = _create_jwt_token(data)

    return token


def authenticate_user(db: Database, state_id: str) -> Optional[str]:
    # Do we have a valid state?
    state = db.query(AuthStateModel).get(state_id)
    if not state:
        return None

    # Is the OAuth flow completed?
    if not state.user_id:
        return None

    user = db.query(UserModel).get(state.user_id)
    if not user:
        return None

    token = create_auth_token(user)

    db.delete(state)
    db.commit()

    return token


JwtAuth = Annotated[str, Depends(get_user)]


def random_str(k: int) -> str:
    return "".join(random.choices(string.ascii_lowercase + string.digits, k=k))


def create_auth_state(db: Database) -> str:
    # Create unique state ID
    state_id = ""
    while True:
        state_id = random_str(32)
        state = db.query(AuthStateModel).get(state_id)
        if not state:
            break

    state = AuthStateModel(
        id=state_id,
        state=random_str(16),
        created_at=datetime.now(tz=timezone.utc),
    )
    db.add(state)
    db.commit()

    return state_id


# def find_auth_state(db: Database, token_id: str) -> Optional[AuthTokenState]:
#    return db.query(AuthTokenState).get(token_id)


def create_unique_user_id(db: Database) -> str:
    # Create unique token ID
    user_id = ""
    while True:
        user_id = random_str(32)
        user = db.query(UserModel).get(user_id)
        if not user:
            break
    return user_id


def process_oauth_user(
    db: Database, user_name: str, user_id: str, provider: str
) -> UserModel:
    # Check if the player is already in the database
    user = (
        db.query(UserModel)
        .filter(UserModel.provider == "itchio")
        .filter(UserModel.login == str(user_id))
        .first()
    )
    if not user:
        # User does not exist yet, create them
        logger.info(f"Creating new player from itch.io user_id: {user_id}")
        user = UserModel(
            id=create_unique_user_id(db),
            login=str(user_id),
            provider=provider,
            created_at=datetime.now(timezone.utc),
            updated_at=datetime.now(timezone.utc),
        )
    else:
        # User already exists, update them
        logger.info(f"Updating an existing player from itch.io user_id: {user_id}")
        user.updated_at = datetime.now(timezone.utc)

    db.add(user)
    db.commit()

    return user


def process_oauth_state(db: Database, user_id: str, state: Optional[str]):
    # Find the token using the state from OAuth
    if state:
        found = db.query(AuthStateModel).filter(AuthStateModel.state == state).first()

        # Update its user id if it exists
        if found:
            found.user_id = user_id
            db.add(found)
            db.commit()
        else:
            logger.warning(
                f"No state token found for user_id: {user_id} state: {state}"
            )


def process_oauth_state_by_id(db: Database, user_id: str, state_id: str):
    # Find the token using the state from cookies
    found = db.query(AuthStateModel).get(state_id)

    # Update its user id if it exists
    if found:
        found.user_id = user_id
        db.add(found)
        db.commit()
    else:
        logger.warning(
            f"No state token found for user_id: {user_id} state_id: {state_id}"
        )


async def oauth_login_itchio(
    config: Config, db: Database, access_token: str, state: Optional[str]
) -> UserModel:
    async with ItchIOClient(config.itchio_base_url, access_token) as client:
        # Get the player information
        profile = await client.get_profile()
        logger.info(f"Received player profile from itch.io: {profile}")

        # Process the login
        user = process_oauth_user(db, profile.name, str(profile.user_id), "itchio")
        process_oauth_state(db, user.id, state)

        return user


async def oauth_login_github(
    config: Config, db: Database, code: str, state: Optional[str]
) -> UserModel:
    async with GitHubOAuthClient(
        config.github_base_url,
        config.oauth_github_client_id,
        config.oauth_github_client_secret,
    ) as oauth_client:
        access_token = await oauth_client.get_access_token(code)

    async with GitHubClient(config.github_api_url, access_token) as client:
        # Get the player information
        profile = await client.get_profile()
        logger.info(f"Received player profile from itch.io: {profile}")

        # Process the login
        user = process_oauth_user(db, profile.name, str(profile.user_id), "github")
        process_oauth_state(db, user.id, state)

        return user
