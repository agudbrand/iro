local options = assert(lake.getOptions())

local mode = options.mode or "debug"
local build_dir = lake.cwd().."/build/"..mode.."/"

options.registerCleaner(function()
	lake.unlink(build_dir)
end)

lake.mkdir(build_dir, {make_parents = true})

local report = assert(options.report)
local recipes = assert(options.recipes)

report.includeDir(lake.cwd().."iro")

local c_files = lake.find("iro/**/*.cpp")

for c_file in c_files:each() do
	local o_file = c_file:gsub("(.-)%.cpp", build_dir.."%1.o")
	local d_file = o_file:gsub("(.-)%.o", "%1.d")

	report.objFile(o_file)

	lake.target(o_file)
		:dependsOn(c_file)
		:recipe(recipes.compiler(c_file, o_file))

	lake.target(d_file)
		:dependsOn(c_file)
		:recipe(recipes.depfile(c_file, d_file, o_file))
end