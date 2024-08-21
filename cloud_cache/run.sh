#!/bin/bash
set -x
sudo podman \
    run \
    --rm \
    --runtime=/usr/local/bin/crun \
    --annotation run.oci.handler=krun \
    --entrypoint /bin/bash \
    -p 33:33 \
    -v /flag:/flag:ro \
    -it js-challenge

