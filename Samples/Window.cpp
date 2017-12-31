// Make sure you export C functions. Use your own macros if you like.
#ifndef ENTRY_EXTERNC
#	ifdef __cplusplus
#		define ENTRY_EXTERNC extern "C"
#	else 
#		define ENTRY_EXTERNC
#	endif
#endif // ENTRY_EXTERNC
#ifndef ENTRY_INTERFACE
#	if defined(_WIN32)
#		define ENTRY_INTERFACE ENTRY_EXTERNC __declspec(dllexport)
#	else
#		define ENTRY_INTERFACE ENTRY_EXTERNC //__attribute__((visibility("default")))
#	endif
#endif // ENTRY_INTERFACE

#include <iostream>
#include "SDL2/include/SDL.h"

static SDL_Window* window;

ENTRY_INTERFACE int Init()
{
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS) != 0) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return 1;
	}
	char str[80];
	strcpy (str,"Antler Testing ");
	//strcat (str,Antler__App_GetEngineVersion());

	window = SDL_CreateWindow(str, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 512, 512, SDL_WINDOW_OPENGL);
	return 0;
}

// Reload() is called once every time a library is reloaded (also the first time).
ENTRY_INTERFACE int Reload()
{
	std::cout << "Reloading library press q to quit." << std::endl;
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS) != 0) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return 1;
	}
	char str[80];
	strcpy (str,"Antler Testing ");

	//strcat (str,Antler__App_GetEngineVersion());

	window = SDL_CreateWindow(str, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 512, 512, SDL_WINDOW_OPENGL);

	return 0;
}

// Update() is called in a loop once loaded.
// You are responsible for delaying the loop.
ENTRY_INTERFACE int Update()
{
	SDL_Event e;
	//Read any events that occured, for now we'll just quit if any event occurs
	while (SDL_PollEvent(&e)){
		//If user closes the window
		if (e.type == SDL_QUIT){
			SDL_Quit();
		}
	}
	SDL_Delay(10);
	return 0;
}

ENTRY_INTERFACE void Unload()
{
	printf("unload\n");
	SDL_DestroyWindow(window);
	SDL_Quit();
}