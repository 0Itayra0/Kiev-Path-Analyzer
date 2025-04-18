cmake_minimum_required(VERSION 3.10)
project(MyProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ImGui + ImPlot
file(GLOB IMGUI_SRC
    "libs/imgui/*.cpp"
    "libs/imgui/backends/imgui_impl_glfw.cpp"
    "libs/imgui/backends/imgui_impl_opengl3.cpp"
)

file(GLOB IMPLOT_SRC
    "libs/implot/*.cpp"
)

set(STB_SRC libs/stb/stb_image.cpp)

add_executable(MyProject
    main.cpp
    ${IMGUI_SRC}
    ${IMPLOT_SRC}
    ${STB_SRC}
)

# Platform-specific setup
if (WIN32)
    message(STATUS "Configuring for Windows")

    # Paths to libraries
    set(GLFW_ROOT "C:/Users/Itayra/Documents/GLFW")
    set(GLEW_ROOT "C:/Users/Itayra/Documents/glew-2.1.0")

    # For GLFW
    find_library(GLFW_LIBRARY
        NAMES glfw3 glfw3.lib
        PATHS "${GLFW_ROOT}/lib-vc2022"
        NO_DEFAULT_PATH
        REQUIRED
    )
    message(STATUS "GLFW library: ${GLFW_LIBRARY}")

    # For GLEW
    find_library(GLEW_LIBRARY
        NAMES glew32 glew32s
        PATHS "${GLEW_ROOT}/lib/Release/x64"
        NO_DEFAULT_PATH
        REQUIRED
    )
    message(STATUS "GLEW library: ${GLEW_LIBRARY}")

    # Include directories
    target_include_directories(MyProject PRIVATE
        "${GLFW_ROOT}/include"
        "${GLEW_ROOT}/include"
    )

    # Link libraries
    target_link_libraries(MyProject
        ${GLFW_LIBRARY}
        ${GLEW_LIBRARY}
        opengl32
    )

    # Auto-copy DLLs and assets
    add_custom_command(TARGET MyProject POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${GLEW_ROOT}/bin/Release/x64/glew32.dll"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/map.png"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/"
        COMMENT "Copying required DLLs and assets to output directory"
    )

else()  # Linux/macOS
    # ... Nah, not yet
endif()

# Common include paths (ImGui, ImPlot, stb)
target_include_directories(MyProject PRIVATE
    libs/imgui
    libs/imgui/backends
    libs/implot
    libs/stb
)