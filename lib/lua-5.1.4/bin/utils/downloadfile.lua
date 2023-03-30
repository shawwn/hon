require("socket")
require("platform")

function normalizePath (path)
	return string.gsub(path, "\\", "/")
end

function findLastOf(s, of, start, plain)
	local ret = nil
	local at = (start or 1) - 1
	while true do
		found = string.find(s, of, at + 1, plain)
		if found == nil then break end
		ret = found
		at = found + 1
	end
	return ret
end

function stripFilename(filepath)
	local pos = findLastOf(filepath, "/")
	if pos == nil then return "" end
	return filepath:sub(1, pos)
end

function getFilenameExtension(filepath)
	local pos = findLastOf(filepath, "/")
	if pos == nil then pos = 1 end
	pos = findLastOf(filepath, ".", pos, true)
	if pos == nil then return "" end
	return filepath:sub(pos)
end

function createDir (dirname)
	local path = normalizePath(dirname)
	path = stripFilename(path)
	if path ~= "" then
		if os.type == "windows" then
			path = string.gsub(path, "/", "\\")
			os.execute("mkdir " .. path .. " 2> NUL")
		else
			os.execute("mkdir " .. path .. " &> /dev/null")
		end
	end
end

function download (host, file)
	file = normalizePath(file)
	if file:find("/") == 1 then file = file:sub(2) end

	print ("download " .. host .. " " .. file)
	local c = assert(socket.connect(host, 80))
	local count = 0    -- counts number of bytes read
	c:send("GET /" .. file .. " HTTP/1.1\r\n")
	c:send("Host: " .. host .. "\r\n")
	c:send("Connection: keep-alive\r\n")
	c:send("Accept: application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/ping,*/*;q=0.5\r\n")
	local ext = getFilenameExtension(file)
	if ext == ".zip" or ext == ".s2z" then
		c:send("Accept-Encoding: gzip,deflate,sdch\r\n")
	else
		c:send("Accept-Encoding: \r\n")
	end
	c:send("Accept-Language: en-US,en;q=0.8\r\n")
	c:send("\r\n")

	local line = "*"
	while line ~= "" do
		local status
		line, status = c:receive("*l")
		print(line)
		if status == "closed" then return end
	end

	createDir(file)
	local out = assert(io.open(file, "wb"))
	while true do
		local data, status = receive(c)
		local s = tostring(data)
		out:write(s)
		count = count + string.len(s)
		if status == "closed" then break end
	end
	out:close()
	c:close()
	print(file, tostring(count) .. " bytes")
end

function receive (connection)
	return connection:receive("*a")
end

download(arg[1], arg[2])

