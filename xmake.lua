add_rules("mode.debug", "mode.release")

add_rules("plugin.compile_commands.autoupdate", { outputdir = "." })
target("gb28181-server")
set_default(true)
set_enabled(true)
set_kind("binary")
add_files("src/*.cpp", "src/event_handler/**.cpp", "src/http/**.cpp", "src/utils/**.cpp", "src/msg_builder/**.cpp")

set_targetdir("./")
--add_cflags("-g")
if is_os("linux") then
	add_defines("LINUX", "FMT_HEADER_ONLY")
	print("current os is linux")
	set_languages("c++11")
	add_cxxflags("-O0", "-Wall", "-g2", "-ggdb")
	add_includedirs("./", "/usr/local/include")

	add_linkdirs("/usr/local/lib")

	add_links(
		-- "iconv",
		"hv",
		"osip2",
		"osipparser2",
		"eXosip2",
		"boost_thread",
		"boost_filesystem"
	)

	add_syslinks("pthread")
elseif is_os("windows") then
	print("current os is Windows")
	add_defines("WIN32", "FMT_HEADER_ONLY")
	add_includedirs("./", "E:/sleeping/3rd_lib/include", "E:/sleeping/code/vcpkg/installed/x64-windows/include")

	add_linkdirs(
		"E:/sleeping/3rd_lib/lib",
		"E:/sleeping/code/vcpkg/installed/x64-windows/lib",
		"E:/sleeping/code/vcpkg/installed/x64-windows/bin"
	)

	add_links(
		"hv",
		"cares",
		"osip2",
		"osipparser2",
		"eXosip",
		"Dnsapi",
		"iconv",
		"boost_thread-vc140-mt",
		"boost_filesystem-vc140-mt"
	)
else
	print("unsuppported os!")
end

