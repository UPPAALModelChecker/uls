#!/usr/bin/env bash
set -e

./build/_deps/utap-src/getlibs/getlibs.sh linux64

cp -R build/_deps/utap-src/local/ local/
