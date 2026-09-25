#!/bin/sh
# Lancia la demo Linux: serve eseguire da qui (working directory = questa cartella),
# cosi' il motore trova assets/ e libengine.so.
cd "$(dirname "$0")"
LD_LIBRARY_PATH=. ./client
