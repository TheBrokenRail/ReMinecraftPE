cmake_minimum_required(VERSION 3.16.0)
project(reminecraftpe-sdl1)

# SDL Build
target_compile_definitions(reminecraftpe-core
    PUBLIC USE_SDL
    PUBLIC USE_SDL_1_2
    PUBLIC HANDLE_CHARS_SEPARATELY
)

# Build
add_executable(reminecraftpe
    main.cpp
    AppPlatform_sdl.cpp
)

# Core
target_link_libraries(reminecraftpe reminecraftpe-core)

# stb_image
target_link_libraries(reminecraftpe stb_image)

# SDL
find_package(SDL REQUIRED)
target_include_directories(reminecraftpe-core PUBLIC "${SDL_INCLUDE_DIR}")
target_link_libraries(reminecraftpe-core PUBLIC "${SDL_LIBRARY}")
if(SDLMAIN_LIBRARY)
    target_link_libraries(reminecraftpe "${SDLMAIN_LIBRARY}")
endif()

# OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(reminecraftpe-core PUBLIC OpenGL::GL)