import asyncio
import contextlib
import logging
from dataclasses import dataclass
from typing import Dict, Optional

from pydantic import BaseModel
from starlette.websockets import WebSocket


logger = logging.getLogger(__name__)


@dataclass
class Endpoint:
    address: str
    port: int


@dataclass
class PendingConnection:
    event: asyncio.Event
    result: Optional[Endpoint]


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


class ConnectionManager:
    def __init__(self):
        self.lock = asyncio.Lock()
        self.connections: Dict[str, ConnectionInfo] = {}

    def add_server(self, server_id: str, websocket: WebSocket):
        self.connections[server_id] = ConnectionInfo(
            websocket=websocket,
            pending_connections={},
        )

    def remove_server(self, server_id: str):
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
        logger.info(
            f"Resolving connection request for server: {server_id} client: {res.id}"
        )

        if server_id not in self.connections:
            return

        conn = self.connections[server_id]
        if res.id not in conn.pending_connections:
            return

        pen = conn.pending_connections[res.id]
        pen.result = Endpoint(
            address=res.address,
            port=res.port,
        )
        pen.event.set()

    async def contact_server(
        self, server_id: str, client_id: str, endpoint: Endpoint
    ) -> Optional[Endpoint]:
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
                address=endpoint.address,
                port=endpoint.port,
            )

            await conn.websocket.send_json(
                {
                    "event": data.__type__,
                    "data": data.model_dump(),
                }
            )
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
