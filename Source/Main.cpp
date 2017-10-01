#include "Entry.h"

#include <iostream>

ENTRY_MAIN()
{
	//Entry_Attach("", "SimpleLoop");
	Entry_Attach("", "SimpleLoop");
	while (Entry_Run()) {
		
	}

	return 0;
}