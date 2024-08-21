local x = 0x1330;
local y = 0x1331;

_ENV[0] = 1;

_ENV[0] = _ENV[0] & (x < y);
_ENV[0] = _ENV[0] & (x ~= y);
_ENV[0] = _ENV[0] & ((x + 1) == y);
_ENV[0] = _ENV[0] & (5 == 5);
_ENV[0] = _ENV[0] + 0x1336;
