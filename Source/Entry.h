#ifndef DEERLYGAMES_ENTRY_H
#define DEERLYGAMES_ENTRY_H

#include <stdint.h>

#	ifdef __cplusplus
#		define ENTRY_EXTERNC extern "C"
#	else 
#		define ENTRY_EXTERNC extern
#	endif

#ifdef __ANDROID__
ENTRY_EXTERNC int entry_main(int argc, char *argv[]);
#	define ENTRY_MAIN ENTRY_EXTERNC int SDL_main
#else 
#	define ENTRY_MAIN int main
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define ENTRY_SILENT	1

const char* Entry_GetPath();

int Entry_Attach(const char* _path);

/// Attach the path of a dynamically loadable gLibrary to the system.
int Entry_AttachExt(const char* _dir, const char* _name, const char* _prefix = "?", const char* _suffix = "?");

/// Runs the specified entry setup. Returns 0 (Requests quiting) or 1 (Keep running).
int Entry_Run(int _flags = 0);

void Entry_WebRun();

/*-------------------------------------------------------*/
/*------------Auto Generated Version Macros -------------*/
/*-------------------------------------------------------*/
#ifndef ENTRY_VERSION_MAJOR
#define ENTRY_VERSION_MAJOR 0
#endif
#ifndef ENTRY_VERSION_MINOR
#define ENTRY_VERSION_MINOR 2
#endif
#ifndef ENTRY_VERSION_PATCH
#define ENTRY_VERSION_PATCH 3
#endif

#define ENTRY_PLATFORM_ANDROID 			0
#define ENTRY_PLATFORM_BSD 				0
#define ENTRY_PLATFORM_DESKTOP			0
#define ENTRY_PLATFORM_LINUX 			0
#define ENTRY_PLATFORM_IOS 				0
#define ENTRY_PLATFORM_MACOS 			0
#define ENTRY_PLATFORM_POSIX			0
#define ENTRY_PLATFORM_PS4 				0
#define ENTRY_PLATFORM_UNIX				0
#define ENTRY_PLATFORM_WINDOWS			0
#define ENTRY_PLATFORM_WIN_UNIVERSAL 	0
#define ENTRY_PLATFORM_WEB				0
#define ENTRY_PLATFORM_XBOXONE			0

/* We figure out the platforms with macro definition statements.*/
#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(__WIN64__)
#	undef 	ENTRY_PLATFORM_WINDOWS
#	define 	ENTRY_PLATFORM_WINDOWS 1
#elif defined(__ANDROID__)
#	include <sys/cdefs.h> // Defines __BIONIC__ and includes android/api-level.h
#	undef 	ENTRY_PLATFORM_ANDROID
#	define 	ENTRY_PLATFORM_ANDROID 1 // __ANDROID_API__
#elif defined(__linux__)
#	undef 	ENTRY_PLATFORM_LINUX
#	define 	ENTRY_PLATFORM_LINUX 1
#elif defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__)
#	undef 	ENTRY_PLATFORM_MACOS
#	define 	ENTRY_PLATFORM_MACOS 1
#elif defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__)
#	undef 	ENTRY_PLATFORM_IOS
#	define 	ENTRY_PLATFORM_IOS 1
#elif defined(EMSCRIPTEN)
#	undef 	ENTRY_PLATFORM_WEB
#	define 	ENTRY_PLATFORM_WEB 1
#elif defined(_DURANGO) || defined(_XBOX_ONE)
#	undef 	ENTRY_PLATFORM_XBOXONE
#	define 	ENTRY_PLATFORM_XBOXONE 1
#endif

#if defined(unix) || defined(__unix) || defined(__unix__) || defined(EMSCRIPTEN)
#	undef	ENTRY_PLATFORM_UNIX
#	define	ENTRY_PLATFORM_UNIX 1
#endif

#if ENTRY_PLATFORM_ANDROID || ENTRY_PLATFORM_LINUX || ENTRY_PLATFORM_MACOS || ENTRY_PLATFORM_WEB
#	undef	ENTRY_PLATFORM_POSIX
#	define	ENTRY_PLATFORM_POSIX 1
#endif

//int AttachLibExt(const char* _name, const char* _prefix = "?", const char* _suffix = "?", const std::string& _dir = "");

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

#endif // DEERLYGAMES_ENTRY_H
