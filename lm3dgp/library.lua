function setup_solution(sln)
    solution(sln)
        configurations("Debug", "Release")
end

function setup_project(proj, ...)
    project(proj)
        kind "ConsoleApp"
        language "c++"

        files {...}
        includedirs {"../include", "../include/GLUS", "../framework"}
        links {"GLUS", "glfw3", "GLEW", "glutil", "glimg"}

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
            buildoptions {"-std=c++11" }
end

