import logging
from datetime import datetime, timezone, timedelta
from typing import List
from uuid import uuid4
from sqlalchemy.orm import Session
from matchmaker.client import Client
from matchmaker.models import Server, BaseModel
from matchmaker.router import Router


logger = logging.getLogger(__name__)
router = Router()


class Endpoint(BaseModel):
    address: str
    port: int


class ServerDto(BaseModel):
    id: str
    name: str


class MessageServerRegister(BaseModel):
    __type__ = "server_register"

    name: str
    endpoint: Endpoint


class MessageServerRegistered(BaseModel):
    __type__ = "server_registered"

    id: str


class MessageServerPing(BaseModel):
    __type__ = "server_ping"

    time: datetime


class MessageServerQuery(BaseModel):
    __type__ = "server_query"

    page: int


class MessageServerList(BaseModel):
    __type__ = "server_list"

    servers: List[ServerDto]
    page: int
    pages: int


class MessageServerRefresh(BaseModel):
    __type__ = "server_refresh"

    endpoint: Endpoint


@router.on_message(MessageServerRegister)
async def ws_register(client: Client, db: Session, msg: MessageServerRegister):
    logger.info(f"Registering server name: {msg.name} address: {msg.endpoint.address}")
    server = Server(
        id=str(uuid4()),
        name=msg.name,
        address=msg.endpoint.address,
        port=msg.endpoint.port,
        owner=client.token,
        last_ping=datetime.now(tz=timezone.utc),
    )
    db.add(server)
    db.commit()

    await client.send(MessageServerRegistered(
        id=server.id,
    ))


@router.on_message(MessageServerPing)
async def ws_ping(client: Client, db: Session, msg: MessageServerPing):
    server = db.query(Server).filter(Server.owner == client.token).first()
    if server:
        server.last_ping = datetime.now(tz=timezone.utc)
        db.add(server)
        db.commit()


@router.on_disconnect()
async def ws_on_disconnect(client: Client, db: Session):
    logger.info("ws_on_disconnect")


def do_server_cleanup(db: Session):
    tp = datetime.now(tz=timezone.utc) - timedelta(seconds=30)
    servers = db.query(Server).filter(Server.last_ping <= tp).all()
    for server in servers:
        logger.info(f"Removing stale server: {server.name} ({server.id})")
        db.delete(server)
        db.commit()
