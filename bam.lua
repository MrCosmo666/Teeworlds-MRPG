CheckVersion("0.5")

Import("configure.lua")
Import("other/sdl/sdl.lua")
Import("other/freetype/freetype.lua")
Import("other/boost/boost.lua")

--- Setup Config -------
config = NewConfig()
config:Add(OptCCompiler("compiler"))
config:Add(OptTestCompileC("stackprotector", "int main(){return 0;}", "-fstack-protector -fstack-protector-all"))
config:Add(OptTestCompileC("minmacosxsdk", "int main(){return 0;}", "-mmacosx-version-min=10.7 -isysroot /Developer/SDKs/MacOSX10.7.sdk"))
config:Add(OptLibrary("zlib", "zlib.h", false))
config:Add(SDL.OptFind("sdl", true))
config:Add(FreeType.OptFind("freetype", true))
config:Add(Boost.OptFind("boost", true))
config:Finalize("config.lua")

generated_src_dir = "src"
generated_icon_dir = "build/icons"
builddir = ""
content_src_dir = "datasrc/"
 
-- data compiler  
function Python(name)
	if family == "windows" then
		-- Python is usually registered for .py files in Windows
		return str_replace(name, "/", "\\")
	end
	return "python " .. name
end

function CHash(output, ...)
	local inputs = TableFlatten({...})

	output = PathJoin(generated_src_dir, Path(output))

	-- compile all the files
	local cmd = Python("scripts/cmd5.py") .. " "
	for index, inname in ipairs(inputs) do
		cmd = cmd .. Path(inname) .. " "
	end

	cmd = cmd .. " > " .. output

	AddJob(output, "cmd5 " .. output, cmd)
	for index, inname in ipairs(inputs) do
		AddDependency(output, inname)
	end
	AddDependency(output, "scripts/cmd5.py")
	return output
end

function ResCompile(scriptfile, compiler)
	scriptfile = Path(scriptfile)
	local output = nil
	if compiler == "cl" then
		output = PathJoin(generated_icon_dir, PathBase(PathFilename(scriptfile)) .. ".res")
		AddJob(output, "rc " .. scriptfile, "rc /fo " .. output .. " " .. scriptfile)
	elseif compiler == "gcc" or compiler == "clang" then
		output = PathJoin(generated_icon_dir, PathBase(PathFilename(scriptfile)) .. ".coff")
		AddJob(output, "windres " .. scriptfile, "windres -i " .. scriptfile .. " -o " .. output)
	end
	AddDependency(output, scriptfile)
	return output
end

function ContentCompile(action, output)
	output = PathJoin(generated_src_dir, Path(output))
	AddJob(
		output,
		action .. " > " .. output,
		Python("datasrc/compile.py") .. " " .. action .. " > " .. output
	)
	AddDependency(output, "datasrc/compile.py")
	AddDependency("datasrc/compile.py", "datasrc/content.py", "datasrc/network.py", "datasrc/datatypes.py")
	return output
end


function GenerateCommonSettings(settings, conf, arch, compiler)

	-- Compile zlib if needed
	local zlib = nil
	if config.zlib.value == 1 then
		settings.link.libs:Add("z")
		if config.zlib.include_path then
			settings.cc.includes:Add(config.zlib.include_path)
		end
	else
		settings.cc.includes:Add("src/engine/external/zlib")
		zlib = Compile(settings, Collect("src/engine/external/zlib/*.c"))
	end

	local md5 = Compile(settings, Collect("src/engine/external/md5/*.c"))
	local wavpack = Compile(settings, Collect("src/engine/external/wavpack/*.c"))
	local png = Compile(settings, Collect("src/engine/external/pnglite/*.c"))
	local json = Compile(settings, Collect("src/engine/external/json-parser/*.c"))

	-- globally available libs
	libs = {zlib=zlib, wavpack=wavpack, png=png, md5=md5, json=json}
end

function GenerateLinuxSettings(settings, conf, arch, compiler)
	if arch == "x86" then
		settings.cc.flags:Add("-msse2") -- for the _mm_pause call
		settings.cc.flags:Add("-m32")
		settings.link.flags:Add("-m32")
	elseif arch == "x86_64" then
		settings.cc.flags:Add("-m64")
		settings.link.flags:Add("-m64")
	elseif arch == "armv7l" then
		-- arm 32 bit
	else
		print("Unknown Architecture '" .. arch .. "'. Supported: x86, x86_64")
		os.exit(1)
	end

	if config.boost.use_discord == true then
		if config.compiler.driver == "clang" then
			settings.cc.flags:Add("-Wno-unused-command-line-argument")
			settings.link.flags:Add("-Wno-unused-command-line-argument")
		end

		settings.cc.flags:Add("-lssl -lcurl -lcrypto")
		settings.link.flags:Add("-lssl -lcurl -lcrypto")
		settings.cc.includes:Add("discord_deps/cpr/include")

	end

	settings.link.libs:Add("mysqlcppconn")
	settings.link.libs:Add("pthread")

	if ExecuteSilent("pkg-config icu-uc icu-i18n") == 0 then
	end		
	
	settings.cc.flags:Add("`pkg-config --cflags icu-uc icu-i18n`")
	settings.link.flags:Add("`pkg-config --libs icu-uc icu-i18n`")

	GenerateCommonSettings(settings, conf, arch, compiler)

	-- Master server, version server and tools
	BuildEngineCommon(settings)
	BuildTools(settings)
	BuildMasterserver(settings)
	BuildVersionserver(settings)

	-- Add requirements for Server & Client
	BuildGameCommon(settings)

	-- Server
	BuildServer(settings)

	-- Client
	settings.link.libs:Add("X11")
	settings.link.libs:Add("GL")
	settings.link.libs:Add("GLU")
	BuildClient(settings)

	-- Content
	BuildContent(settings)
end

function GenerateSolarisSettings(settings, conf, arch, compiler)
	settings.link.libs:Add("socket")
	settings.link.libs:Add("nsl")

	GenerateLinuxSettings(settings, conf, arch, compiler)
end

function GenerateWindowsSettings(settings, conf, target_arch, compiler)
	if compiler == "cl" then
		if (target_arch == "x86" and arch ~= "ia32") or
		   (target_arch == "x86_64" and arch ~= "ia64" and arch ~= "amd64") then
			print("Cross compiling is unsupported on Windows.")
			os.exit(1)
		end
		settings.cc.flags:Add("/wd4244", "/wd4577", "/EHsc")
	elseif compiler == "gcc" or config.compiler.driver == "clang" then
		-- if target_arch ~= "x86" and target_arch ~= "x86_64" then
			print("Need CL compiler use Visual Studio '")
			os.exit(1)
		-- end


	end

	local icons = SharedIcons(compiler)

	-- Required libs
	settings.cc.flags_cxx:Add("-std=c++17")
	
	settings.link.libs:Add("gdi32")
	settings.link.libs:Add("user32")
	settings.link.libs:Add("ws2_32")
	settings.link.libs:Add("ole32")
	settings.link.libs:Add("shell32")
	settings.link.libs:Add("advapi32")

	-- Added includes
	settings.cc.includes:Add("other\\icu\\include")
	settings.cc.includes:Add("other\\mysql\\include")
	settings.cc.includes:Add("other\\boost\\include")

	-- Added other lib
	if config.compiler.driver == "cl" then
		if target_arch == "x86" or target_arch == "x64_86" then
			settings.link.libpath:Add("other/icu/vc/lib32")
			settings.link.libs:Add("icudt")
			settings.link.libs:Add("icuin")
			settings.link.libs:Add("icuuc")

			settings.link.libpath:Add("other/mysql/windows/x32")
			settings.link.libs:Add("mysqlcppconn")
		elseif target_arch == "x64" or target_arch == "x86_64" then
			settings.link.libpath:Add("other/icu/vc/lib64")
			settings.link.libs:Add("icudt")
			settings.link.libs:Add("icuin")
			settings.link.libs:Add("icuuc")

			settings.link.libpath:Add("other/mysql/windows/x64")
			settings.link.libs:Add("mysqlcppconn")
		end
	end

	-- Other settings
	GenerateCommonSettings(settings, conf, target_arch, compiler)

	-- Master server, version server and tools 
	BuildEngineCommon(settings)
	BuildMasterserver(settings)
	BuildVersionserver(settings)
	BuildTools(settings)

	-- Add requirements for Server & Client
	BuildGameCommon(settings)

	-- Server
	local server_settings = settings:Copy()
	server_settings.link.extrafiles:Add(icons.server)
	BuildServer(server_settings)

	-- Client
	settings.link.extrafiles:Add(icons.client)
	settings.link.libs:Add("opengl32")
	settings.link.libs:Add("glu32")
	settings.link.libs:Add("winmm")
	BuildClient(settings)

	-- Content
	BuildContent(settings)
end

function SharedCommonFiles()
	-- Shared game files, generate only once

	if not shared_common_files then
		local network_source = ContentCompile("network_source", "generated/protocol.cpp")
		local network_header = ContentCompile("network_header", "generated/protocol.h")
		AddDependency(network_source, network_header, "src/engine/shared/protocol.h")

		local nethash = CHash("generated/nethash.cpp", "src/engine/shared/protocol.h", "src/game/tuning.h", "src/game/gamecore.cpp", network_header)
		shared_common_files = {network_source, nethash}
	end

	return shared_common_files
end

function SharedServerFiles()
	-- Shared server files, generate only once

	if not shared_server_files then
		local server_content_source = ContentCompile("server_content_source", "generated/server_data.cpp")
		local server_content_header = ContentCompile("server_content_header", "generated/server_data.h")
		AddDependency(server_content_source, server_content_header)
		shared_server_files = {server_content_source}
	end

	return shared_server_files
end

function SharedClientFiles()
	-- Shared client files, generate only once

	if not shared_client_files then
		local client_content_source = ContentCompile("client_content_source", "generated/client_data.cpp")
		local client_content_header = ContentCompile("client_content_header", "generated/client_data.h")
		AddDependency(client_content_source, client_content_header)
		shared_client_files = {client_content_source}
	end

	return shared_client_files
end

shared_icons = {}
function SharedIcons(compiler)
	if not shared_icons[compiler] then
		local server_icon = ResCompile("other/icons/teeworlds_srv_" .. compiler .. ".rc", compiler)
		local client_icon = ResCompile("other/icons/teeworlds_" .. compiler .. ".rc", compiler)
		shared_icons[compiler] = {server=server_icon, client=client_icon}
	end
	return shared_icons[compiler]
end

function BuildEngineCommon(settings)
	settings.link.extrafiles:Merge(Compile(settings, Collect("src/engine/shared/*.cpp", "src/base/*.c")))
end

function BuildGameCommon(settings)
	settings.link.extrafiles:Merge(Compile(settings, Collect("src/game/*.cpp"), SharedCommonFiles()))
end


function BuildClient(settings, family, platform)
	config.sdl:Apply(settings)
	config.freetype:Apply(settings)
	
	local client = Compile(settings, Collect("src/engine/client/*.cpp"))
	
	local game_client = Compile(settings, CollectRecursive("src/game/client/*.cpp"), SharedClientFiles())
	local game_editor = Compile(settings, Collect("src/game/editor/*.cpp"))
	
	Link(settings, "teeworlds", libs["zlib"], libs["md5"], libs["wavpack"], libs["png"], libs["json"], depends, client, game_client, game_editor)
end

function BuildServer(settings, family, platform)

	local server

	if config.boost.use_discord == true then
		server = Compile(settings, Collect("src/engine/server/*.cpp", "src/teeother/components/*.cpp", "src/teeother/system/*.cpp", 
			"src/teeother/sleepy_discord/*.cpp", "src/teeother/tl/*.cpp", "discord_deps/cpr/include/cpr/*.cpp"))
	else
		server = Compile(settings, Collect("src/engine/server/*.cpp", "src/teeother/components/*.cpp", "src/teeother/system/*.cpp", "src/teeother/tl/*.cpp"))
	end

	local game_server = Compile(settings, CollectRecursive("src/game/server/*.cpp"), SharedServerFiles())

	return Link(settings, "teeworlds_srv", libs["zlib"], libs["md5"], libs["json"], server, game_server)
end

function BuildTools(settings)
	local tools = {}
	for i,v in ipairs(Collect("src/tools/*.cpp", "src/tools/*.c")) do
		local toolname = PathFilename(PathBase(v))
		tools[i] = Link(settings, toolname, Compile(settings, v), libs["zlib"], libs["md5"], libs["wavpack"], libs["png"])
	end
	PseudoTarget(settings.link.Output(settings, "pseudo_tools") .. settings.link.extension, tools)
end

function BuildMasterserver(settings)
	return Link(settings, "mastersrv", Compile(settings, Collect("src/mastersrv/*.cpp")), libs["zlib"], libs["md5"])
end

function BuildVersionserver(settings)
	return Link(settings, "versionsrv", Compile(settings, Collect("src/versionsrv/*.cpp")), libs["zlib"], libs["md5"])
end

function BuildContent(settings)
	local content = {}
	table.insert(content, CopyToDir(settings.link.Output(settings, "data"), CollectRecursive(content_src_dir .. "*.png", content_src_dir .. "*.wv", content_src_dir .. "*.ttf", content_src_dir .. "*.txt", content_src_dir .. "*.map", content_src_dir .. "*.rules", content_src_dir .. "*.json")))
	PseudoTarget(settings.link.Output(settings, "content") .. settings.link.extension, content)
end

-- create all targets for specified configuration & architecture
function GenerateSettings(conf, arch, builddir, compiler)
	local settings = NewSettings()

	-- Set compiler if explicitly requested
	if compiler == "gcc" then
		SetDriversGCC(settings)
	elseif compiler == "clang" then
		SetDriversClang(settings)
	elseif compiler == "cl" then
		SetDriversCL(settings)
	else
		-- apply compiler settings
		config.compiler:Apply(settings)
		compiler = config.compiler.driver
	end
	
	if conf ==  "debug" then
		settings.debug = 1
		settings.optimize = 0

		if config.boost.use_discord == true then
			settings.cc.defines:Add("CONF_DEBUG", "CONF_DISCORD")	
		else	
			settings.cc.defines:Add("CONF_DEBUG")
		end
	else
		settings.debug = 0
		settings.optimize = 1

		if config.boost.use_discord == true then
			settings.cc.defines:Add("CONF_RELEASE", "CONF_DISCORD")	
		else	
			settings.cc.defines:Add("CONF_RELEASE")
		end
	end
	
	-- Generate object files in {builddir}/objs/
	settings.cc.Output = function (settings_, input)
		-- strip 
		input = input:gsub("^src/", "")
		input = input:gsub("^" .. generated_src_dir .. "/", "")
		return PathJoin(PathJoin(builddir, "objs"), PathBase(input))
	end
	
	-- Build output files in {builddir}
	settings.link.Output = function (settings_, input)
		return PathJoin(builddir, PathBase(input) .. settings_.config_ext)
	end
	
	settings.cc.includes:Add("src")
	settings.cc.includes:Add(generated_src_dir)
	
	if family == "windows" then
		GenerateWindowsSettings(settings, conf, arch, compiler)
	elseif family == "unix" then
		if platform == "solaris" then
			GenerateSolarisSettings(settings, conf, arch, compiler)
		else -- Linux, BSD
			GenerateLinuxSettings(settings, conf, arch, compiler)
		end
	end

	return settings
end

-- String formatting wth named parameters, by RiciLake http://lua-users.org/wiki/StringInterpolation
function interp(s, tab)
	return (s:gsub('%%%((%a%w*)%)([-0-9%.]*[cdeEfgGiouxXsq])',
			function(k, fmt)
				return tab[k] and ("%"..fmt):format(tab[k]) or '%('..k..')'..fmt
			end))
end

function CopyToDir(dst, ...)
	local output = {}
	for filename in TableWalk({...}) do
		table.insert(output, CopyFile(PathJoin(dst, string.sub(filename, string.len(content_src_dir)+1)), filename))
	end
	return output
end

function split(str, sep)
	local vals = {}
	str:gsub("([^,]+)", function(val) table.insert(vals, val) end)
	return vals
end

-- Supported archtitectures: x86, amd64, ppc, ppc64
if ScriptArgs['arch'] then
	archs = split(ScriptArgs['arch'])
else
	if arch == "ia32" then
		archs = {"x86"}
	elseif arch == "ia64" or arch == "amd64" then
		archs = {"x86_64"}
	else
		archs = {arch}
	end
end

if ScriptArgs['conf'] then
	confs = split(ScriptArgs['conf'])
else
	confs = {"debug"}
end

if ScriptArgs['compiler'] then
	compiler = ScriptArgs['compiler']
else
	compiler = nil
end

if ScriptArgs['builddir'] then
	builddir = ScriptArgs['builddir']
end

targets = {client="teeworlds", server="teeworlds_srv",
           versionserver="versionsrv", masterserver="mastersrv",
           tools="pseudo_tools", content="content"}

subtargets = {}
for t, cur_target in pairs(targets) do
	subtargets[cur_target] = {}
end
for a, cur_arch in ipairs(archs) do
	for c, cur_conf in ipairs(confs) do
		cur_builddir = interp(builddir, {platform=family, arch=cur_arch, target=cur_target, conf=cur_conf, compiler=compiler})
		local settings = GenerateSettings(cur_conf, cur_arch, cur_builddir, compiler)
		for t, cur_target in pairs(targets) do
			table.insert(subtargets[cur_target], PathJoin(cur_builddir, cur_target .. settings.link.extension))
		end
	end
end
for cur_name, cur_target in pairs(targets) do
	-- Supertarget for all configurations and architectures of that target
	PseudoTarget(cur_name, subtargets[cur_target])
end

PseudoTarget("game", "client", "server", "content")
DefaultTarget("game")
