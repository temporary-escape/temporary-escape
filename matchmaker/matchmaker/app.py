import json
import logging
from fastapi import FastAPI, WebSocket
from starlette.websockets import WebSocketDisconnect

from matchmaker.db import SessionLocal, Base, engine
from matchmaker.models import BaseModel
from matchmaker.client import Client
from matchmaker.router import Router
from matchmaker.api.servers import router as server_router, do_server_cleanup
from matchmaker.utils import repeat_every

app = FastAPI()

logger = logging.getLogger(__name__)

router = Router()
router.add(server_router)

db = SessionLocal()
Base.metadata.create_all(bind=engine)


class MessageHello(BaseModel):
    __type__ = "hello"

    description: str


@app.on_event("startup")
@repeat_every(seconds=10, logger=logger, wait_first=True)
def periodic_cleanup():
    do_server_cleanup(db)


@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()

    client = Client(websocket)
    logger.info(f"Client connected: {client}")

    await client.send(MessageHello(
        description="Temporary Escape matchmaker server",
    ))

    while True:
        try:
            data = await websocket.receive_text()

            msg = json.loads(data)
            await router.handle(client, db, msg)

        except WebSocketDisconnect as e:
            logger.warning(f"Client disconnected: {client}")
            break

    await router.disconnect(client, db)
