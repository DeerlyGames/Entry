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

//#include "AntlerDebug.h"
#include "EntryPublic.h"


// Init() is only called the first time a library is loaded.
ENTRY_INTERFACE int Init()
{
//	Antler_Debug_Console("hello world\n");
//	malloc(32);
	return 0;
}

// Reload() is called once every time a library is reloaded.
ENTRY_INTERFACE int Update(Entry_State* _state)
{

/*	if (_state != NULL)
		memcpy(&state, _state, sizeof(state));
	_state = &state;
*/	
/*	SDL_Event e;
	//Read any events that occured, for now we'll just quit if any event occurs
	while (SDL_PollEvent(&e)){
		//If user closes the window
		if (e.type == SDL_QUIT){
			SDL_Quit();
		}
	}  */


//	SDL_Delay(10);
	return 0;
}

#if _WIN32 && !defined(DllMain)
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif // WIN32_LEAN_AND_MEAN
#	include <Windows.h>
__pragma(warning(push))
__pragma(warning(disable:4100))

BOOL WINAPI DllMain(
HINSTANCE hinstDLL,  // handle to DLL module
DWORD fdwReason,     // reason for calling function
LPVOID lpReserved)  // reserved
{
	// Perform actions based on the reason for calling.
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		// Initialize once for each new process.
		// Return FALSE to fail DLL load.
		break;

	case DLL_THREAD_ATTACH:
		// Do thread-specific initialization.
		break;

	case DLL_THREAD_DETACH:
		// Do thread-specific cleanup.
		break;

	case DLL_PROCESS_DETACH:
		// Perform any necessary cleanup.
		break;
	}
	return TRUE;  // Successful DLL_PROCESS_ATTACH.
}

BOOL WINAPI _DllMainCRTStartup(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	return DllMain(hinstDLL, fdwReason, lpReserved);
}

__pragma(warning(pop))

#endif
