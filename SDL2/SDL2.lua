-----------------------------------------------------
---------------------  SDL2  ------------------------
-----------------------------------------------------
	project ("SDL2")
	location 	"../Build"
	targetname 	"SDL2"
	kind 		"StaticLib"
	language 	"C"
	warnings 	"Off"

	defines{"DECLSPEC=",
			"SDL_VIDEO_RENDER_D3D11=0",
			"SDL_VIDEO_RENDER_OGL=0",
			"SDL_VIDEO_RENDER_ES2=0",
			"SDL_VIDEO_RENDER_OGL_ES2=0"}

	includedirs{ "include",
				 "src" }

	files { "src/*.c",
			"src/atomic/*.c",
			"src/audio/*.c",
			"src/cpuinfo/*.c",
			"src/dynapi/*.c",
			"src/events/*.c",
			"src/file/*.c",
			"src/haptic/*.c",
			"src/joystick/*.c",
			"src/libm/*.c",
			"src/power/*.c",
			"src/render/*.c",
			"src/render/software/*.c",
			"src/stdlib/*.c",
			"src/thread/*.c",
			"src/timer/*.c",
			"src/video/*.c",
			"src/video/dummy/*.c" }

	filter {"system:android" }
		defines { "GL_GLEXT_PROTOTYPES" }
		files { "src/audio/android/*.c",
				"src/core/android/*.c",
				"src/filesystem/android/*.c", -- Maybe not needed
				"src/haptic/dummy/*.c",
				"src/joystick/android/*.c",
				"src/loadso/dlopen/*.c",
				"src/loadso/dummy/*.c",
				"src/power/android/*.c", -- Maybe not needed
				"src/thread/pthread/*.c",
				"src/timer/unix/*.c", -- Maybe not needed
				"src/video/android/*.c"
				}
