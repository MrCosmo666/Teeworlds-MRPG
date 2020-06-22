Boost = {
	basepath = PathDir(ModuleFilename()),

	OptFind = function (name, required)
		local check = function(option, settings)
			option.value = false
			option.use_discord = false

			if platform == "win32" then
				option.value = true
			elseif platform == "win64" then
				option.value = true
			elseif platform == "macosx" and string.find(settings.config_name, "32") then
				option.value = true
			elseif platform == "macosx" and string.find(settings.config_name, "64") then
				option.use_discord = true
				option.value = true
			elseif platform == "linux" and arch == "ia32" then
				option.value = true
			elseif platform == "linux" and arch == "amd64" then
				option.value = true
			end
		end

		local apply = function(option, settings)
			settings.cc.includes:Add(Boost.basepath .. "/include")
		end

		local save = function(option, output)
			output:option(option, "value")
			output:option(option, "use_discord")
		end

		local display = function(option)

			if option.value == true then
				if option.use_discord == true then
					return "enabled discord bot. compile conf=discord_release|discord_debug"
				end

				return "using bundled includes"
			else
				if option.required then
					return "not found (required)"
				else
					return "not found (optional)"
				end
			end
		end

		local o = MakeOption(name, 0, check, save, display)
		o.Apply = apply
		o.required = required
		return o
	end
}
