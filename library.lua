function setup_solution(sln)
    solution(sln)
        configurations("Debug", "Release")
end

function setup_project(proj, ...)
    project(proj)
        kind "ConsoleApp"
        language "c++"

        files {...}
        includedirs {"../include/GLUS"}
        links {"GLUS", "glfw3", "glew"}
        libdirs {"../lib"}

        configuration "Debug"
            defines {"DEBUG", "_DEBUG"}
            flags "Symbols"
            targetname (proj .. "Debug")

        configuration "Release"
            defines {"RELEASE", "NDEBUG"};
            flags {"OptimizeSpeed", "ExtraWarnings"};
            targetname (proj .. "Release")
                  
        configuration "linux"
            links {"GL", "GLU", "X11"}

        configuration "macosx"
            linkoptions {"-framework OpenGL"}

        configuration {"gmake"}
            buildoptions {"-std=c++11" }
end

