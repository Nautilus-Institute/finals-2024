FROM ubuntu:22.04 AS base

RUN apt-get update && apt-get install -y kmod dkms

FROM base AS builder

RUN apt-get update && apt-get install -y kmod build-essential wget curl git gcc make


FROM base AS runner

COPY kern/jscache.ko /lib/modules/jscache.ko
COPY quickjs-2019-07-09/libquickjs.so /lib/libquickjs.so
COPY quickjs-2019-07-09/entrypoint.bin /entrypoint.bin
COPY e.sh /e.sh

CMD ["/e.sh"]