#!/usr/bin/env bash

set -e

DIR=$1
HOST=$2
TARGET_DIR=/home/ubuntu/temporary-escape

cmake --build "${DIR}" --target package
TAR_GZ_PATH=$(find "${DIR}/release" -name "*.tar.gz" -print0 | xargs -r -0 ls -1 -t)
TAR_GZ_FILE=$(basename "${TAR_GZ_PATH}")

scp "${TAR_GZ_PATH}" ${HOST}:~/
ssh "${HOST}" "rm -rf \"${TARGET_DIR}\" && mkdir \"${TARGET_DIR}\" && tar -xzf ~/${TAR_GZ_FILE} -C ${TARGET_DIR}"

# ssh "${HOST}" "${TARGET_DIR}/TemporaryEscape server"
