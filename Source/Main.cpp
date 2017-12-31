#include "Entry.h"
#ifdef __EMSCRIPTEN__
#	include "Emscripten.h"
#endif
#include <iostream>
#include <stdio.h>
#include <string>


ENTRY_MAIN(int argc, char *argv[])
{
	if(argc!=2){
		printf( "Please pass the dynamic library to be embedded as an argument.\n" );
		return 1;
	}

	Entry_Attach(argv[1]);

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(Entry_WebRun, 60, 1);

#else
	while(Entry_Run()){

	}
#endif

	return 0;
}