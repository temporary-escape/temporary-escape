#!/usr/bin/env bash

set -e

source ./venv/bin/activate

python3 -m uvicorn matchmaker.app:app --reload --port 3000 --proxy-headers --log-config=log_conf.yaml
