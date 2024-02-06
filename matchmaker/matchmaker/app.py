import logging
from fastapi import FastAPI, APIRouter
from fastapi.responses import RedirectResponse

from matchmaker.db import Base, engine

from matchmaker.routers.auth_v1 import router as auth_router_v1
from matchmaker.routers.servers_v1 import router as servers_router_v1

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
api.include_router(auth_router_v1)
api.include_router(servers_router_v1)
app.include_router(api)

logger = logging.getLogger(__name__)

Base.metadata.create_all(bind=engine)


@app.get("/api")
async def redirect_to_docs():
    return RedirectResponse(url="/api/docs")
