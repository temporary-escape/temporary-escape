VENV = ./venv
PYTHON3 = $(VENV)/bin/python3
PYINSTALLER = $(VENV)/bin/pyinstaller
SHELL := /bin/bash
IMAGE_NAME := temporary-escape-matchmaker
#IMAGE_VERSION := $(shell git describe --always --tags)
IMAGE_VERSION := latest
SERVER_HOST = server.temporaryescape.org

.ONESHELL:
.PHONY: install format build image deploy

./venv:
	python3 -m venv ./venv

install: ./venv
	$(PYTHON3) -m pip install .

build: ./venv
	$(PYINSTALLER) ./matchmaker/main.py -n matchmaker --distpath ./dist

format: ./venv
	$(PYTHON3) -m black ./matchmaker

image:
	docker build . -t $(IMAGE_NAME):$(IMAGE_VERSION)

deploy: image
	docker save $(IMAGE_NAME):$(IMAGE_VERSION) | gzip | ssh -C $(SERVER_HOST) docker load
	ssh $(SERVER_HOST) bash -c '"docker compose -f ./deploy/docker-compose.yaml down || true"'
	scp -r ./deploy $(SERVER_HOST):~/
	ssh $(SERVER_HOST) bash -c '"set -e; if [ ! -e ./deploy/traefik/acme.json ]; then echo \"{}\" > ./deploy/traefik/acme.json; chmod 600 ./deploy/traefik/acme.json; fi"'
	ssh $(SERVER_HOST) docker compose -f ./deploy/docker-compose.yaml up -d
	ssh $(SERVER_HOST) docker image prune --all --force
