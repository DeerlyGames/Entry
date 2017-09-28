#include "Entry.h"

#include <iostream>

ENTRY_MAIN( int argc, char* argv[] )
{
	//Entry_Attach("", "SimpleLoop");
	Entry_Attach("", "SimpleLoop");
	while (Entry_Run()) {

	}
	//Entry_Attach("Player", "", ".aelib");

	return 0;
}