VENV = ./venv
PYTHON3 = $(VENV)/bin/python3
PYINSTALLER = $(VENV)/bin/pyinstaller
SHELL := /bin/bash
IMAGE_NAME := temporary-escape/temporary-escape-matchmaker
#IMAGE_VERSION := $(shell git describe --always --tags)
IMAGE_VERSION := latest
SERVER_HOST = server.temporaryescape.org

.ONESHELL:
.PHONY: install lint format build image deploy

./venv:
	python3 -m venv ./venv

install: ./venv
	$(PYTHON3) -m pip install .

build: ./venv
	$(PYINSTALLER) ./matchmaker/main.py -n matchmaker --distpath ./dist

lint: ./venv
	$(PYTHON3) -m black ./matchmaker --check
	$(PYTHON3) -m ruff ./matchmaker

format: ./venv
	$(PYTHON3) -m black ./matchmaker
	$(PYTHON3) -m ruff ./matchmaker --fix

image:
	docker build . -t $(IMAGE_NAME):$(IMAGE_VERSION)

run: ./venv
	$(PYTHON3) -m matchmaker.main --log-config=log_conf.yaml

deploy: image
	docker save $(IMAGE_NAME):$(IMAGE_VERSION) | ssh -C $(SERVER_HOST) k3s ctr images import -
	scp -r ./chart $(SERVER_HOST):~/
	ssh $(SERVER_HOST) "KUBECONFIG=/etc/rancher/k3s/k3s.yaml helm upgrade --install matchmaker ./chart/ --values ~/.temporary-escape-chart-values.yaml --namespace matchmaker --create-namespace --wait"
	ssh $(SERVER_HOST) "KUBECONFIG=/etc/rancher/k3s/k3s.yaml kubectl --namespace matchmaker rollout restart deployment matchmaker-api"
