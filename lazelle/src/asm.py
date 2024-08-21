# extremely janky arm to ihex

from keystone import *
from intelhex import IntelHex
import sys, io

with open(sys.argv[1], "rb") as f:
	data = f.read().strip()

# remove comments
data = b"\n".join([x for x in data.split(b"\n") if b";" not in x])

ks = Ks(KS_ARCH_ARM, KS_MODE_ARM)
encoding, count = ks.asm(data)
print("%s = %s (number of statements: %u)" %(data, encoding, count))

ih = IntelHex()
ih.start_addr = 0
for x in range(len(encoding)):
    ih[x] = encoding[x]

sio = io.StringIO()
ih.write_hex_file(sio)
res = sio.getvalue()
with open(sys.argv[2], "wb") as f:
    f.write(res.encode("latin-1"))
