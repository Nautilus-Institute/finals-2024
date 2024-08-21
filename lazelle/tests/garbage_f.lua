local x = 0x1330;
for i in ({1, 1, 1, 1, 2, 3, 4}) do
	x = x + 1;
end
_ENV[0] = x;
_ENV[1] = x;