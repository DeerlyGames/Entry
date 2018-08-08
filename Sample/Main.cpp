#include "Entry.h"
#ifdef __EMSCRIPTEN__
#	include "Emscripten.h"
#endif
#include <stdio.h>
#include <string>
#include "EntryPublic.h"
#include "SDL.h"

#undef main
ENTRY_MAIN(int argc, char *argv[])
{
	if(argc!=2){
		printf( "Please pass the dynamic library to be embedded as an argument.\n" );
		return 1;
	}
	
	Startup();

	Entry_Attach(argv[1]);

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(Entry_WebRun, 60, 1);

#else
	while(Entry_Run()){
		SDL_Event e;
		//Read any events that occured, for now we'll just quit if any event occurs
		while (SDL_PollEvent(&e)){
			//If user closes the window
			if (e.type == SDL_QUIT){
				SDL_Quit();
			}
		}  
		SDL_Delay(10);
	}
#endif

	return 0;
}