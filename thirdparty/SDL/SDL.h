#pragma once

#ifdef USE_SDL_1_2

// SDL 1.2
#include <SDL.h>

#else

// SDL 2
#ifdef _WIN32
#pragma comment(lib, "SDL2.lib")
#endif
#include <SDL.h>
#include <SDL_syswm.h>

#endif