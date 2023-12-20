add_rules("mode.debug", "mode.release")

target("gb28181-server")
    set_kind("binary")
    add_files(
        "src/*.cpp",
        "src/event_handler/**.cpp",
        "src/http/**.cpp",
        "src/utils/**.cpp",
        "src/msg_builder/**.cpp"
    )
    set_languages("c++11")

    set_targetdir("./")
    add_cxxflags("-O0", "-Wall", "-g2", "-ggdb")
    --add_cflags("-g")

    add_includedirs(
        "./",
        "/usr/local/include"
    )

    add_linkdirs(
        "/usr/local/lib"
    )

    add_links(
        "iconv",
        "hv",
        "osip2", 
        "osipparser2",
        "eXosip2",
        "boost_thread",
        "boost_filesystem"
    )

    add_syslinks(
        "pthread"
    )

target("gb28181-client")
    set_kind("binary")
    add_files("src/client/src/**.cpp")
    set_languages("c++11")

    set_targetdir("./")
    add_cxxflags("-O0", "-Wall", "-g2", "-ggdb")
    --add_cflags("-g")

    add_includedirs(
        "./",
        "/usr/local/include"
    )

    add_linkdirs(
        "/usr/local/lib"
    )

    add_links(
        "hv",
        "osip2", 
        "osipparser2",
        "eXosip2",
        "boost_thread"
    )

    add_syslinks(
        "pthread"
    )

target("test-gb28181")
    set_kind("binary")
    add_files("src/test/*.cpp", "src/utils/*.cpp")
    set_languages("c++11")

    set_targetdir("./")
    add_cxxflags("-O0", "-Wall", "-g2", "-ggdb")
    --add_cflags("-g")

    add_includedirs(
        "./",
        "/usr/local/include"
    )

    add_linkdirs(
        "/usr/local/lib"
    )

    add_links(
        "iconv",
        "gtest",
        "gtest_main"
    )

    add_syslinks(
        "pthread"
    )

