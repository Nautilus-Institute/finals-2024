#!/bin/bash
set -e

PLATFORM="idk"
UNAME=$(uname)
echo $UNAME
if [ "$UNAME" == 'Linux' ]; then
   PLATFORM="linux"
elif [ "$UNAME" == 'Darwin' ]; then
   PLATFORM="macosx"
fi

cp replace_luac.c lua-5.3.5/src/luac.c
cd lua-5.3.5
make clean
make $PLATFORM
cd ../