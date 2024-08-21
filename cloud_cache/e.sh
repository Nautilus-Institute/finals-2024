#!/bin/bash

set -e

insmod /lib/modules/jscache.ko

chmod 777 /dev/jscache

exec /entrypoint.bin



