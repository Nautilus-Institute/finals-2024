local a = 1;
local b = a + 3;
local c = b | 5;
d = (c << 1) + b - 0x100
local e = d ~ 0x11
local f = e + 0x1337 + 225;
_ENV[0] = f;