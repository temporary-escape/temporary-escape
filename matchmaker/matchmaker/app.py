import json
import logging
from fastapi import FastAPI, WebSocket, APIRouter
from fastapi.responses import RedirectResponse
from starlette.websockets import WebSocketDisconnect

from matchmaker.db import SessionLocal, Base, engine
from matchmaker.models import BaseModel

from matchmaker.routers.auth import router as auth_router
from matchmaker.routers.servers import router as servers_router

app = FastAPI(
    title="Temporary Escape Game Matchmaker API",
    version="0.1.0",
    contact={
        "name": "Matus Novak",
        "email": "email@matusnovak.com",
    },
    openapi_url="/api/openapi.json",
    docs_url="/api/docs",
)
api = APIRouter(prefix="/api")
api.include_router(auth_router)
api.include_router(servers_router)
app.include_router(api)

logger = logging.getLogger(__name__)

# = Router()
# router.add(server_router)

Base.metadata.create_all(bind=engine)


@app.get("/api")
async def redirect_to_docs():
    return RedirectResponse(url="/api/docs")


"""

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
"""
