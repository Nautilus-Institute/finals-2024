#!/bin/bash

# Path to the kernel headers for the target VM
LOCAL_KERNEL_PATH=/opt/linux/linux-6.2.14

sudo docker build \
    --target builder \
    -t js-builder .

sudo docker \
    run \
    -v $LOCAL_KERNEL_PATH:/linux-6.2.14 \
    -v $(pwd)/:/c/ \
    -w /c/kern \
    --entrypoint /bin/bash \
    -it js-builder \
    -c "make clean && make"

sudo docker \
    run \
    -v $(pwd)/:/c/ \
    -w /c/quickjs-2019-07-09 \
    --entrypoint /bin/bash \
    -it js-builder \
    -c "make clean && make entrypoint.bin"

