input = _ENV[0];
flag = _ENV[1];

local result = 0;

for x = 0, 0x20, 1 do
    result = result | (input[x] ~= flag[x]);
    result = result | (input[x] > flag[x]);
end

_ENV[0] = result;
