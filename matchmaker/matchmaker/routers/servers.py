import asyncio
import contextlib
import dataclasses
import json
import logging
import uuid
from dataclasses import dataclass
from datetime import datetime, timezone, timedelta
from math import ceil
from typing import Annotated, List, Dict, Optional

from fastapi import APIRouter, Response, Depends
from fastapi.responses import JSONResponse
from fastapi.exceptions import HTTPException
from pydantic import BaseModel
from starlette.websockets import WebSocket, WebSocketDisconnect

from matchmaker.auth import authenticate_user, validate_user, JwtAuth
from matchmaker.db import Database, get_db
from matchmaker.models import Server
from matchmaker.utils import repeat_every

logger = logging.getLogger(__name__)
router = APIRouter(prefix="/servers", tags=["Server Management"])


@dataclass
class StunResult:
    address: str
    port: int


@dataclass
class PendingConnection:
    event: asyncio.Event
    result: Optional[StunResult]


@dataclass
class ConnectionInfo:
    websocket: WebSocket
    pending_connections: Dict[str, PendingConnection]


class EventConnectionRequest(BaseModel):
    __type__: str = "ConnectionRequest"

    id: str
    address: str
    port: int


class EventConnectionResponse(BaseModel):
    __type__: str = "ConnectionResponse"

    id: str
    address: str
    port: int


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


class ConnectionManager:
    def __init__(self):
        self.lock = asyncio.Lock()
        self.connections: Dict[str, ConnectionInfo] = {}

    def add_server(self, server_id: str, websocket: WebSocket):
        self.connections[server_id] = ConnectionInfo(
            websocket=websocket,
            pending_connections={},
        )

    def remove_server(self, server_id:  str):
        if server_id not in self.connections:
            return

        self._notify_pending_connections(server_id)
        del self.connections[server_id]

    def _notify_pending_connections(self, server_id: str):
        if server_id in self.connections:
            for _, pen in self.connections[server_id].pending_connections.items():
                pen.event.set()

    def _erase_pending_connection(self, server_id: str, client_id: str):
        if server_id not in self.connections:
            return
        del self.connections[server_id].pending_connections[client_id]

    def contact_client(self, server_id: str, res: EventConnectionResponse):
        logger.info(f"Resolving connection request for server: {server_id} client: {res.id}")

        if server_id not in self.connections:
            return

        conn = self.connections[server_id]
        if res.id not in conn.pending_connections:
            return

        pen = conn.pending_connections[res.id]
        pen.result = StunResult(
            address=res.address,
            port=res.port,
        )
        pen.event.set()

    async def contact_server(self, server_id: str, client_id: str, req: ServerConnectModel) -> Optional[StunResult]:
        if server_id not in self.connections:
            return None

        # Create an event that will be triggered from the websocket request
        conn = self.connections[server_id]
        event = asyncio.Event()
        conn.pending_connections[client_id] = PendingConnection(
            event=event,
            result=None,
        )

        try:
            # Send an event to the server that we want to connect to it
            data = EventConnectionRequest(
                id=client_id,
                address=req.address,
                port=req.port,
            )

            await conn.websocket.send_json({
                "event": data.__type__,
                "data": data.model_dump(),
            })
        except Exception as e:
            # Something went wrong while sending the event
            logger.warning(f"Failed to send event to server: {server_id} error: {e}")
            self._erase_pending_connection(server_id, client_id)
            return None

        # Wait for the event to happen
        with contextlib.suppress(asyncio.TimeoutError):
            await asyncio.wait_for(event.wait(), timeout=2.0)

            # Timeout reached?
            if not event.is_set():
                logger.warning(f"Timeout waiting for response from server: {server_id}")
                self._erase_pending_connection(server_id, client_id)
                return None

        # Did the server disappear?
        if server_id not in self.connections:
            logger.warning(f"The server no longer exists: {server_id}")
            self._erase_pending_connection(server_id, client_id)
            return None

        # Grab the result
        res = self.connections[server_id].pending_connections[client_id].result

        # Ease ourselves from the pending connection list
        self._erase_pending_connection(server_id, client_id)

        return res


connection_manager = ConnectionManager()


@router.post("/register", summary="Register a public server")
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


@router.get("", summary="List public servers")
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


@router.post("/{server_id}/connect", summary="Request connection to the server")
async def connect_server(body: ServerConnectModel, server_id: str, user: JwtAuth, db: Database) -> ServerConnectModel:
    server = db.query(Server).get(server_id)
    if not server:
        raise HTTPException(
            status_code=404,
            detail="Server not found",
        )

    stun = await connection_manager.contact_server(server_id, str(user.username), body)
    if stun is None:
        raise HTTPException(
            status_code=409,
            detail="Failed to contact the server",
        )

    return ServerConnectModel(
        address=stun.address,
        port=stun.port,
    )


@router.websocket("/{server_id}/events")
async def websocket_endpoint(server_id: str, db: Database, websocket: WebSocket):
    global connection_manager

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

            if not isinstance(data, dict) or "event" not in data or not isinstance(data["event"], str):
                break

            event = data["event"]
            # logger.info(f"Received event from the connected server of type: {event}")

            if "data" not in data or not isinstance(data["data"], dict):
                break

            if event == EventConnectionResponse.__type__:
                connection_manager.contact_client(server_id, EventConnectionResponse(**data["data"]))

            else:
                logger.error(f"Received unknown event type: {event} from server: {server_id}")

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
