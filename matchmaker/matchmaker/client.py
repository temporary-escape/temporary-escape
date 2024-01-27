from fastapi import WebSocket
from uuid import uuid4


class Client:
    def __init__(self, ws: WebSocket):
        self.ws = ws
        self.token = str(uuid4())

    async def send(self, msg: any):
        data = {
            "type": msg.__type__,
            "data": msg.model_dump(),
        }
        await self.ws.send_json(data, mode="text")

