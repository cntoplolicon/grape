function setup_solution(sln)
    solution(sln)
        configurations("Debug", "Release")
end

function pkg_config(libs)
    local libs_str = table.concat(libs, " ")
    buildoptions {"`pkg-config --cflags " .. libs_str .. "`"}
    linkoptions {"`pkg-config --libs " .. libs_str .. "`"}
end

function setup_project(proj, ...)
    project(proj)
        kind "ConsoleApp"
        language "c++"

        files {...}
        includedirs {"../include", "../include/GLUS", "../framework"}
        links {"GLUS"}

        flags {"ExtraWarnings", "FatalWarnings"}

        configuration "Debug"
            defines {"DEBUG", "_DEBUG"}
            flags {"Symbols"}
            targetname (proj .. "Debug")

        configuration "Release"
            defines {"RELEASE", "NDEBUG"};
            flags {"OptimizeSpeed"};
            targetname (proj .. "Release")
                  
        configuration "linux"
            links {"GL", "GLU", "X11", "Xxf86vm", "pthread", "Xrandr", "Xi"}
            libdirs {"../lib/linux"}

        configuration "macosx"
            linkoptions {"-framework OpenGL"}
            defines {"MACOSX"}
            libdirs {"../lib/osx"}

        configuration {"gmake"}
            buildoptions {"-std=c++11"}
            linkoptions {"-lGLUS"}   -- workaround of the stupid linking order issue of g++
            pkg_config {"glfw3", "glew"}
end

