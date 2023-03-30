
argc = table.getn(arg)
value = _G[arg[1]]

if argc == 1 then
	for k, v in pairs(value) do
		print(k)
	end
elseif argc == 2 then
	for k, v in pairs(value) do
		if k == arg[2] then
			print(k)
			info = debug.getinfo(v)
			print(info.short_src)
			for k2, v2 in pairs(info) do
				print(k2)
			end
		end
	end
end
