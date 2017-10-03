#include "Entry.h"
#ifdef __EMSCRIPTEN__
#	include "Emscripten.h"
#endif
#include <iostream>

ENTRY_MAIN(int argc, char *argv[])
{
	//Entry_Attach("", "SimpleLoop");
	Entry_Attach("", "SimpleLoop");
	
#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(Entry_WebRun, 60, 1);

#else
	while(Entry_Run()){

	}
#endif

	return 0;
}