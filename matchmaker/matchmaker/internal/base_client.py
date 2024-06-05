import logging
from types import TracebackType
from typing import Optional, Type, Generic, TypeVar
import aiohttp
from typing_extensions import Self
from pydantic import BaseModel


class ItchIOProfileResponse(BaseModel):
    name: str
    user_id: int


T = TypeVar("T")


class BaseHttpClient(Generic[T]):
    def __init__(self, base_url: str) -> None:
        self.logger = logging.getLogger(self.__class__.__qualname__)

        trace_config = aiohttp.TraceConfig()
        trace_config.on_request_end.append(self.on_request_end)
        trace_config.on_request_exception.append(self.on_request_exception)

        self.session = aiohttp.ClientSession(base_url, trace_configs=[trace_config])

    async def on_request_end(
        self,
        session: aiohttp.ClientSession,  # noqa: ARG002
        context,
        params: aiohttp.TraceRequestEndParams,
    ):
        log_func = (
            self.logger.error if params.response.status >= 400 else self.logger.info
        )

        log_func(
            f"{params.response.method} {params.response.url} {params.response.status}"
        )

    async def on_request_exception(
        self,
        session: aiohttp.ClientSession,  # noqa: ARG002
        context,  # noqa: ARG002
        params: aiohttp.TraceRequestExceptionParams,
    ):
        self.logger.error(f"{params.method} {params.url} -", params.exception)

    async def __aenter__(self) -> Self:
        await self.session.__aenter__()
        return self

    async def __aexit__(
        self,
        exc_type: Optional[Type[BaseException]],
        exc_val: Optional[BaseException],
        exc_tb: Optional[TracebackType],
    ):
        await self.session.__aexit__(exc_type, exc_val, exc_tb)
