import logging
import aiohttp
from pydantic import BaseModel

from matchmaker.internal.github_client import BaseHttpClient

logger = logging.getLogger(__name__)


class ItchIOProfileResponse(BaseModel):
    name: str
    user_id: int


class ItchIOClient(BaseHttpClient["ItchIOClient"]):
    def __init__(self, base_url: str, access_token: str):
        super().__init__(base_url)
        self.headers = {"Authorization": f"Bearer {access_token}"}

    async def get_profile(self) -> ItchIOProfileResponse:
        async with self.session.get("/api/1/key/me", headers=self.headers) as response:
            response.raise_for_status()
            data = await response.json()

            if "errors" in data:
                raise aiohttp.ClientError(" ".join(data["errors"]))

            try:
                res = ItchIOProfileResponse(
                    name=data["user"]["username"],
                    user_id=data["user"]["id"],
                )
            except Exception as e:
                logger.exception("thw fuck", exc_info=e)
                raise e
            return res
