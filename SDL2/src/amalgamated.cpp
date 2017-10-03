#define DECLSPEC 
#define SDL_VIDEO_RENDER_D3D11 0
#define SDL_VIDEO_RENDER_OGL 0
#define SDL_VIDEO_RENDER_ES2 0
#define SDL_VIDEO_RENDER_OGL_ES2 0

#include "SDL.c"
#include "SDL_assert.c"
#include "SDL_error.c"
#include "SDL_hints.c"
#include "SDL_log.c"
#include "atomic/SDL_atomic.c"
#include "atomic/SDL_spinlock.c"
#include "dynapi/SDL_dynapi.c"
#include "file/SDL_rwops.c"

#if defined(EMSCRIPTEN)
#include "filesystem/emscripten/SDL_sysfilesystem.c"
#endif