from typing import Callable
from sqlalchemy.orm import Session
from matchmaker.client import Client


class Router:
    def __init__(self):
        self.handlers = {}
        self.disconnect_handlers = []

    def add_handler(self, func: Callable, msg: any):
        self.handlers[msg.__type__] = lambda c, d, m: func(c, d, msg(**m))

    async def handle(self, client: Client, db: Session, msg: dict):
        if "type" not in msg or "data" not in msg:
            raise Exception(f"Malformed message: {msg}")

        msg_type = msg["type"]
        if msg_type not in self.handlers:
            raise Exception(f"No such handler for type: {msg_type}")

        await self.handlers[msg_type](client, db, msg["data"])

    async def disconnect(self, client: Client, db: Session):
        for handler in self.disconnect_handlers:
            await handler(client, db)

    def on_message(self, msg: any):
        def inner(func: Callable):
            self.add_handler(func, msg)
            return func

        return inner

    def on_disconnect(self):
        def inner(func: Callable):
            self.disconnect_handlers.append(func)
            return func

        return inner

    def add(self, other: 'Router'):
        for name, func in other.handlers.items():
            self.handlers[name] = func
        for func in other.disconnect_handlers:
            self.disconnect_handlers.append(func)
