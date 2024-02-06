FROM ubuntu:22.04 AS builder

RUN apt-get update && \
    apt-get install -y python3 python3-pip python3-venv make

WORKDIR /build

COPY ./pyproject.toml /build/

RUN python3 -m venv ./venv && \
    . ./venv/bin/activate && \
    python3 -m pip install .

COPY ./matchmaker /build/matchmaker/

RUN . ./venv/bin/activate && \
    pyinstaller ./matchmaker/main.py -n matchmaker --distpath /build/dist

FROM ubuntu:22.04

COPY --from=builder /build/dist/matchmaker/ /app/
COPY ./log_conf.yaml /app/

EXPOSE 8000

ENV DATABASE_URL="sqlite:////app/data/data.db"
ENV WEB_PORT=8000
ENV WEB_HOST=0.0.0.0

WORKDIR /app
VOLUME /app/data
ENTRYPOINT ["/app/matchmaker"]
