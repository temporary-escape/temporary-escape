import logging
from fastapi import FastAPI, WebSocket
from starlette.websockets import WebSocketDisconnect

app = FastAPI()


logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()

    logger.info(f"Client connected from: {websocket.client.host}:{websocket.client.port}")
    while True:
        try:
            data = await websocket.receive_text()
            logger.info(f"Received: {data}")
            await websocket.send_text(f"Message text was: {data} from: {websocket.client.host}:{websocket.client.port}")
            await websocket.send_text(f"And this happened too!")
        except WebSocketDisconnect as e:
            logger.error(f"Client disconnected")
            break
