local function getAssemblyFiles(directory, is_windows)
	if is_windows then
		handle = io.popen("dir " .. directory .. " /B /A-D")
	end

	t = {}
	for f in handle:lines() do
		if path.hasextension(f, ".dll") then
			if string.find(f, "System.") then
				table.insert(t, f)
			end
		end
	end

	handle:close()
	return t
end

function linkAppReferences()
	local antDir = os.getenv("ANT_DIR")
	local monoLibsPath = path.join(antDir, "AntPlay", "mono", "lib", "mono", "4.5")
	local is_windows = os.istarget('windows')

	if is_windows then
		monoLibsPath = monoLibsPath:gsub("/", "\\")
	end

	-- NOTE: We HAVE to use libdirs, using the full path in links won't work
	--				this is a known issue with Visual Studio...
	libdirs { monoLibsPath }
	links { "Ant-ScriptCore" }

	for k, v in ipairs(getAssemblyFiles(monoLibsPath, is_windows)) do
		print("Adding reference to: " .. v)
		links { v }
	end
end