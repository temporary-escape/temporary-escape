#!/usr/bin/env bash

set -e

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

rm /tmp/matchmaker || true
ln -s "${SCRIPT_DIR}" /tmp/matchmaker

if [ ! -d "${SCRIPT_DIR}/certs" ]; then
  rm -rf "${SCRIPT_DIR}/certs"
  mkdir "${SCRIPT_DIR}/certs"

  openssl req \
    -x509 \
    -nodes \
    -days 3650 \
    -subj "/C=US/ST=/L=/O=/OU=/CN=matchmaker.lan/emailAddress=" \
    -newkey rsa:2048 \
    -keyout "${SCRIPT_DIR}/certs/server.key" \
    -out "${SCRIPT_DIR}/certs/server.crt"
fi

traefik "--configFile=${SCRIPT_DIR}/traefik-local.yaml"
