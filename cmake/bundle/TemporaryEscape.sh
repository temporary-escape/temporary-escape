#!/bin/sh

ABSPATH=$(cd "$(dirname "$0")"; pwd -P)
exec "${ABSPATH}/../Resources/TemporaryEscape" "$@"
