import json
import logging
import uuid
from datetime import datetime, timezone, timedelta
from math import ceil
from typing import List

from fastapi import APIRouter, Response
from fastapi.exceptions import HTTPException
from pydantic import BaseModel
from starlette.websockets import WebSocket, WebSocketDisconnect

from matchmaker.auth import validate_user, JwtAuth
from matchmaker.db import Database, get_db
from matchmaker.internal.connections import EventConnectionResponse, connection_manager
from matchmaker.models import Server
from matchmaker.utils import repeat_every, HTTPError

logger = logging.getLogger(__name__)
router = APIRouter(prefix="/v1/servers", tags=["Server Management"])


class RegisterServerModel(BaseModel):
    name: str
    version: str


class PageModel(BaseModel):
    page: int
    total: int
    pages: int


class ServerModel(BaseModel):
    id: uuid.UUID
    name: str
    version: str


class ServerConnectModel(BaseModel):
    address: str
    port: int


class ServerPageModel(PageModel):
    items: List[ServerModel]


@router.post(
    "",
    summary="Register a public server",
    status_code=200,
    responses={
        401: {
            "description": "Not authenticated",
            "model": HTTPError,
        },
    },
)
async def register_server(
    body: RegisterServerModel, user: JwtAuth, db: Database
) -> ServerModel:
    logger.info(f"Registering server name: '{body.name}'")
    server = Server(
        id=str(uuid.uuid4()),
        name=body.name,
        version=body.version,
        address=None,
        port=None,
        owner=str(user.username),
        last_ping=datetime.now(tz=timezone.utc),
    )
    db.add(server)
    db.commit()

    return ServerModel(
        id=server.id,
        name=server.name,
        version=server.version,
    )


"""
@router.put("/{server_id}/ping", summary="Ping healthcheck from the server")
async def ping_server(server_id: str, user: JwtAuth, db: Database):
    logger.debug(f"Ping server id: '{server_id}'")

    server = db.query(Server).get(server_id)
    if not server:
        raise HTTPException(
            status_code=404,
            detail="Server not found",
        )

    if server.owner != str(user.username):
        raise HTTPException(
            status_code=403,
            detail="You are not owner of this server",
        )

    server.last_ping = datetime.now(tz=timezone.utc)
    db.add(server)
    db.commit()

    return Response(status_code=201)
"""


@router.get(
    "",
    summary="List public servers",
    status_code=200,
    responses={
        401: {
            "description": "Not authenticated",
            "model": HTTPError,
        },
    },
)
async def list_servers(
    user: JwtAuth, db: Database, page: int = 1, limit: int = 10
) -> ServerPageModel:
    def to_model(server: Server) -> ServerModel:
        return ServerModel(
            id=uuid.UUID(server.id),
            name=server.name,
            version=server.version,
        )

    total = db.query(Server).count()

    page = page - 1
    if limit > 20:
        limit = 20

    servers = db.query(Server).limit(limit).offset(page * limit)

    return ServerPageModel(
        page=page + 1,
        total=total,
        pages=int(ceil(float(total) / float(limit))),
        items=[to_model(s) for s in servers],
    )


@router.post(
    "/{server_id}/connect",
    summary="Request connection to the server",
    status_code=200,
    responses={
        401: {
            "description": "Not authenticated",
            "model": HTTPError,
        },
        404: {
            "description": "Server not found",
            "model": HTTPError,
        },
        409: {
            "description": "Failed to contact the server",
            "model": HTTPError,
        },
    },
)
async def connect_server(
    body: ServerConnectModel, server_id: str, user: JwtAuth, db: Database
) -> ServerConnectModel:
    server = db.query(Server).get(server_id)
    if not server:
        raise HTTPException(
            status_code=404,
            detail="Server not found",
        )

    endpoint = await connection_manager.contact_server(
        server_id, str(user.username), body
    )
    if endpoint is None:
        raise HTTPException(
            status_code=409,
            detail="Failed to contact the server",
        )

    return ServerConnectModel(
        address=endpoint.address,
        port=endpoint.port,
    )


@router.delete(
    "/{server_id}",
    summary="Delete a registered server",
    status_code=201,
    responses={
        401: {
            "description": "Not authenticated",
            "model": HTTPError,
        },
        404: {
            "description": "Server not found",
            "model": HTTPError,
        },
        403: {
            "description": "You are not the owner of this server",
            "model": HTTPError,
        },
    },
)
async def delete_server(server_id: str, user: JwtAuth, db: Database):
    server = db.query(Server).get(server_id)
    if not server:
        raise HTTPException(
            status_code=404,
            detail="Server not found",
        )

    if server.owner != str(user.username):
        raise HTTPException(
            status_code=403,
            detail="You are not the owner of this server",
        )


@router.websocket("/{server_id}/events")
async def websocket_endpoint(server_id: str, db: Database, websocket: WebSocket):
    await websocket.accept()
    authorization = websocket.cookies.get("Authorization")
    if authorization is None:
        await websocket.close(3401)
        return

    try:
        user = validate_user(authorization)
    except HTTPException as e:
        await websocket.close(e.status_code)
        return

    server = db.query(Server).get(server_id)
    if not server:
        await websocket.close(3404)
        return

    if server.owner != str(user.username):
        await websocket.close(3403)
        return

    logger.info(f"Server websocket connected id: {server_id}")
    connection_manager.add_server(server_id, websocket)

    while True:
        try:
            text = await websocket.receive_text()
            data = json.loads(text)

            if (
                not isinstance(data, dict)
                or "event" not in data
                or not isinstance(data["event"], str)
            ):
                break

            event = data["event"]
            # logger.info(f"Received event from the connected server of type: {event}")

            if "data" not in data or not isinstance(data["data"], dict):
                break

            if event == EventConnectionResponse.__type__:
                connection_manager.contact_client(
                    server_id, EventConnectionResponse(**data["data"])
                )

            else:
                logger.error(
                    f"Received unknown event type: {event} from server: {server_id}"
                )

        except WebSocketDisconnect:
            logger.warning(f"Server websocket disconnected id: {server_id}")
            break

        except Exception as e:
            logger.warning(f"Server websocket id: {server_id} error: {e}")
            break

    connection_manager.remove_server(server_id)


@router.on_event("startup")
@repeat_every(seconds=10, logger=logger, wait_first=True)
def cleanup_stale_servers():
    with get_db() as db:
        tp = datetime.now(tz=timezone.utc) - timedelta(seconds=30)
        servers = db.query(Server).filter(Server.last_ping <= tp).all()

        # logger.info(f"Found {len(servers)} stale servers, cleaning them now...")
        for server in servers:
            logger.info(f"Removing stale server: {server.name} ({server.id})")
            db.delete(server)
            db.commit()
