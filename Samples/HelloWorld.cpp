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
#include <stdlib.h>
#include <stdio.h>
#define malloc(x) my_malloc(x)

void * my_malloc(size_t nbytes)
{
   printf("my_malloc\n");
   return "32";
}


#include <iostream>

// Init() is only called the first time a library is loaded.
ENTRY_INTERFACE int Init()
{
	std::cout << "Hello World" << std::endl;
	malloc(32);
	return 0;
}

// Reload() is called once every time a library is reloaded.
ENTRY_INTERFACE int Reload()
{
	return 1;
}