import uvicorn

from matchmaker.app import app
from matchmaker.config import get_config


def main():
    config = get_config()
    uvicorn.run(
        app,
        port=config.web_port,
        host=config.web_host,
        log_level=config.log_level,
        log_config="log_conf.yaml",
    )


if __name__ == "__main__":
    main()
