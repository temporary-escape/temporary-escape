#!/usr/bin/env bash

set -e

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
TARGET_DIR="${SCRIPT_DIR}/temp"

if [ -z "$1" ]; then
    echo "You must provide path to the tar.gz file"
    exit 1
fi

rm -rf "${TARGET_DIR}"
mkdir -p "${TARGET_DIR}/opt/temporary-escape"

tar -xzf "$1" -C "${TARGET_DIR}/opt/temporary-escape"

cp -r "${SCRIPT_DIR}/root/." "${TARGET_DIR}/"

DESTINATION=$(echo "$1" | sed 's/.tar.gz/.AppImage/g')
appimagetool "${TARGET_DIR}" "${DESTINATION}"
