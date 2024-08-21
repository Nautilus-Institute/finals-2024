function a()
    return 1;
end

_ENV[0] = 1;

_ENV[0] = _ENV[0] & a();

m = 1;
function b(x, y)
    return x + y;
end

_ENV[0] = _ENV[0] + b(1, 2) + m;

_ENV[0] = _ENV[0] + (1 << 12) + (3 << 8) + (3 << 4) + 2;

print(_ENV[0])