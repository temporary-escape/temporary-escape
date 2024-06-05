import logging

import aiohttp
from pydantic import BaseModel

from matchmaker.internal.base_client import BaseHttpClient


class GitHubProfileResponse(BaseModel):
    name: str
    user_id: int


class GitHubOAuthClient(BaseHttpClient["GitHubOAuthClient"]):
    def __init__(self, base_url: str, client_id: str, client_secret: str):
        super().__init__(base_url)
        self.client_id = client_id
        self.client_secret = client_secret

    async def get_access_token(self, code: str) -> str:
        body = {
            "client_id": self.client_id,
            "client_secret": self.client_secret,
            "code": code,
        }
        headers = {
            "Accept": "application/json",
        }

        async with self.session.post(
            "/login/oauth/access_token", json=body, headers=headers
        ) as response:
            response.raise_for_status()
            data = await response.json()

            if "error" in data:
                raise aiohttp.ClientError(data["error"])

            logging.info(f"data: {data}")
            return data["access_token"]


class GitHubClient(BaseHttpClient["GitHubClient"]):
    def __init__(self, base_url: str, access_token: str):
        super().__init__(base_url)
        self.headers = {
            "Authorization": f"Bearer {access_token}",
            "Accept": "application/json",
        }

    async def get_profile(self) -> GitHubProfileResponse:
        async with self.session.get("/user", headers=self.headers) as response:
            response.raise_for_status()
            data = await response.json()

            return GitHubProfileResponse(
                user_id=data["id"],
                name=data["login"],
            )
