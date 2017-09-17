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
#		define ENTRY_INTERFACE ENTRY_EXTERNC
#	endif
#endif // ENTRY_INTERFACE

#include <iostream>

// Reload() is called once every time a library is reloaded (also the first time).
ENTRY_INTERFACE int Reload()
{
	std::cout << "Reloading library press q to quit." << std::endl;
	return 0;
}

// Update() is called in a loop once loaded.
// You are responsible for delaying the loop.
ENTRY_INTERFACE int Update()
{
//	std::cout << "Update" << std::endl;
	return 0;
}