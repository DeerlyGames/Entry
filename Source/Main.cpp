#include "Entry.h"

#include <iostream>

ENTRY_MAIN( int argc, char* argv[] )
{
	std::cout << Entry_GetPath() << std::endl;
	//Entry_Attach("", "SimpleLoop");
	Entry_Attach("C:/Users/michael/Google Drive/Rust/Entry-RustSample/target/release", "SimpleLoop");
	while (Entry_Run()) {

	}
	//Entry_Attach("Player", "", ".aelib");

	return 0;
}