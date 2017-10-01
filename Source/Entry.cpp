#include "Entry.h"

#include <algorithm>
#include <fstream>
#include <map>
#include <ostream>
#include <string>
#include <string.h>

#include <iostream>

#if ENTRY_PLATFORM_WINDOWS
#	ifndef _CRT_SECURE_NO_WARNINGS
#		define _CRT_SECURE_NO_WARNINGS
#	endif // _CRT_SECURE_NO_WARNINGS
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif // WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#	ifndef NOMINMAX
#		define NOMINMAX
#	endif // NOMINMAX
#	include <Windows.h>
#	include <TimeAPI.h>
#	include <locale>
#	include <codecvt>
#	include <vector>

#pragma comment(lib, "Winmm.lib")
// DWORD timeGetTime(void);

std::wstring utf8toUtf16(const std::string & str)
{
	using std::runtime_error;
	using std::string;
	using std::vector;
	using std::wstring;

	if (str.empty())
		return wstring();

	size_t charsNeeded = ::MultiByteToWideChar(CP_UTF8, 0,
		str.data(), (int)str.size(), NULL, 0);
	if (charsNeeded == 0)
		throw runtime_error("Failed converting UTF-8 string to UTF-16");
#pragma warning( push )
#pragma warning( disable : 4267)
	vector<wchar_t> buffer(charsNeeded);
	int charsConverted = ::MultiByteToWideChar(CP_UTF8, 0,
		str.data(), (int)str.size(), &buffer[0], buffer.size());
	if (charsConverted == 0)
		throw runtime_error("Failed converting UTF-8 string to UTF-16");
#pragma warning( pop ) 
	return wstring(&buffer[0], charsConverted);
}

#else
#	include <fcntl.h>
#	include <dlfcn.h> /*dlopen*/
#	include <pthread.h>
#   include <pwd.h> /* getpwuid */
#	include <stdlib.h>
#	include <sys/time.h>
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <unistd.h>
#endif

#if ENTRY_PLATFORM_LINUX
#	include <errno.h> // Perhaps not needed errno
#	include <linux/limits.h>
#	include <sys/inotify.h>
#elif ENTRY_PLATFORM_MACOS
#	include <atomic>
#	include <dirent.h>
#	include <mach-o/dyld.h>
#	include <sys/event.h>
#elif ENTRY_PLATFORM_WEB
#	define PATH_MAX	256
#endif

#ifndef APIENTRY
#	if defined(__WIN32__)
#		define APIENTRY __stdcall
#	else
#		define APIENTRY
#	endif
#endif // APIENTRY

#define ENTRY_PROC(retVal, FuncName, Params)	\
	typedef retVal (APIENTRY * PTR_##FuncName) Params; \
	PTR_##FuncName FuncName = NULL;

ENTRY_PROC(int, Init, (void))
ENTRY_PROC(int, Update, (void))
ENTRY_PROC(int, Reload, (void))
static void* gLibrary;
static std::string gLibName;
static bool gNotifyEnabled = false;
static int gFlags = 0;


//////////////////////////////////////////////////////////////////////
// Start: String Functions
std::string StringAppend(const std::string& _str) {
	std::string res = _str;
	if (!(res.length() == 0) && res.at(res.length() - 1) != '/')
		res.append("/");
	return res;
}

std::string StringReplace(const std::string& _str, const std::string& _before, const std::string& _after) {
	size_t pos = 0;
	std::string retString = _str;
	while ((pos = retString.find(_before.c_str(), pos)) != std::string::npos) {
		retString.replace(pos, _before.length(), _after.c_str());
		pos += _after.length();
	}
	return retString.c_str();
}

std::string RemoveTrailingSlash(const std::string& _str)
{
	std::string res = StringReplace(_str, "\\", "/");
	if (!(res.length() == 0) && res.at(res.length() - 1) == '/')
		res.resize(res.length() - 1);
	return res;
}


// End: String Functions
//////////////////////////////////////////////////////////////////////


bool FileCopy(const std::string& _fromName, const std::string& _toName)
{
	std::ifstream  src(_fromName.c_str(), std::ios::binary);
	std::ofstream  dst(_toName.c_str(), std::ios::binary);

	dst << src.rdbuf();
	return false;
}

bool FileDelete(const std::string& _path)
{
#if ENTRY_PLATFORM_WINDOWS
	return DeleteFileW(utf8toUtf16(_path).c_str()) != 0;
#else
	return remove(_path.c_str()) == 0;
#endif
}

int FileExists(const char* _path) {
	//	AE_ASSERT(!_path.IsEmpty());
#if ENTRY_PLATFORM_WINDOWS
	DWORD attr = GetFileAttributesW(utf8toUtf16(_path).c_str());
	if (attr == 0xFFFFFFFF)
	{
		switch (GetLastError())
		{
		case ERROR_FILE_NOT_FOUND:
		case ERROR_PATH_NOT_FOUND:
		case ERROR_NOT_READY:
		case ERROR_INVALID_DRIVE:
			return false;
		default:
			return false;
			//	handleLastErrorImpl(_path);
		}
	}
	return true;
#else
	struct stat st;
	return ((stat(_path, &st) != -1) && (st.st_mode & S_IFREG));
#endif
}

uint64_t FileSize(const char* _path)
{
#if ENTRY_PLATFORM_WINDOWS
#	pragma warning( push )
#	pragma warning( disable : 4996)
#endif // ENTRY_PLATFORM_WINDOWS
	FILE* file = fopen(_path, "rb");
#if ENTRY_PLATFORM_WINDOWS
#	pragma warning( pop )
#endif // ENTRY_PLATFORM_WINDOWS
	if (file) {
		fseek(file, 0, SEEK_END);
		uint64_t sizeWhole = ftell(file);
		fseek(file, 0, SEEK_SET); // seek back to beginning of file
		fclose(file);
		return sizeWhole;
	}
	else {
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////
// Start: Library Functions

void* LoadLib(void* _handle, const std::string& _path) {
	_handle = 0;
#if ENTRY_PLATFORM_WINDOWS
	_handle = (void*)::LoadLibraryW(utf8toUtf16(_path).c_str());
#else // ENTRY_PLATFORM_POSIX
	_handle = (void *)dlopen(_path.c_str(), RTLD_LOCAL | RTLD_LAZY);
	//    loadError = (const char *) dlerror();
#endif
	return _handle;
}

void* LoadFunction(void* _handle, const char* _symbol) {
	void *pfn = 0;
	if (_handle) {
#if ENTRY_PLATFORM_WINDOWS
		pfn = (void*)GetProcAddress((HMODULE)_handle, _symbol);
#else // AE_PLATFORM_POSIX
		pfn = dlsym(_handle, _symbol);
		//    loadError = (const char *) dlerror();
#endif
	}
	return pfn;
}

bool HasFunction(void* _handle, const char* _symbol) {
	return LoadFunction(_handle, _symbol) != 0;
}

void DestroyLib(void* _handle) {
	if (_handle)
	{
#if ENTRY_PLATFORM_POSIX
		dlclose(_handle);
#elif ENTRY_PLATFORM_WINDOWS || AE_PLATFORM_WINDOWSPHONE
		::FreeLibrary((HMODULE)_handle);
#endif
		_handle = NULL;
	}
}

// End: Library Functions
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Start: FileWatcher & Thread

#if ENTRY_PLATFORM_MACOS
class RefCounted
{
public:
	RefCounted() :
		counter(0)
	{}

	void Duplicate() const {
		++counter;
	}

	void Release() const{
		if(--counter == 0) delete this;
	}

protected:
//	(ReferenceCounted)
	/// Destroys the RefCountedObject.	

	virtual ~RefCounted(){}

private:
	class AtomicInteger{
	public:
		AtomicInteger(){ counter = 0; }

		explicit AtomicInteger(int _val){ counter = _val; }

		~AtomicInteger(){}

		int GetValue() const{ return counter.load(); }

		int operator ++ () { return ++counter; }

		int operator -- () { return --counter; }
		
		std::atomic<int> counter;		
	};

	mutable AtomicInteger counter;	
};
class DirIter
{
public:
	DirIter() :
	pimpl(0)
	{}

	DirIter(const std::string& _dir) :
		path(_dir),
		pimpl(new Implementation(_dir))
	{
		path = pimpl->Get();
	//	file = path;
	}
	DirIter(const DirIter& _iter) :
		path(_iter.path),
		pimpl(_iter.pimpl)
	{
		if (pimpl)
		{
			pimpl->Duplicate();
		//	_file = _path;
		}
	}

	~DirIter(){
		if (pimpl) pimpl->Release();
	}

	bool HasNext() const{
		if (GetName().empty()) {
			return false;
		}
		return true;
	}
	
	DirIter& Next(){
		if(pimpl){
			path = pimpl->Next();
		}
		return *this;
	}

	const std::string& GetName() const{
		return path;
	}

	DirIter& operator = (const DirIter& _it){
		if (pimpl) pimpl->Release();
		pimpl = _it.pimpl;
		if (pimpl)
		{
			pimpl->Duplicate();
			path = _it.path;
			//file = path;
		}
		return *this;		
	}

	//DirIter& operator = (const std::string& _dir);

	bool operator == (const DirIter& _iterator) const{
		return GetName() == _iterator.GetName();		
	}

	bool operator != (const DirIter& _iterator) const{
		return GetName() != _iterator.GetName();
	}

private:
	std::string path;

	class Implementation : public RefCounted
	{
	public:
		Implementation(const std::string& _path) :
			dir(0)
		{
			dir = opendir(_path.c_str());
			if (!dir){ printf("Unable to opendir for DirectoryIterator\n"); }

			Next();
		}

		~Implementation(){
			if(dir) closedir(dir);
		}

		const std::string& Get() const {
			return current;
		}

		const std::string& Next() {
			do
			{
				struct dirent* pEntry = readdir(dir);
				if (pEntry)
					current = pEntry->d_name;
				else
					current.clear();
			}
			while (current == "." || current == "..");
			return current;
		}

	private:
		DIR*        dir;
		std::string current;
	};
	Implementation* pimpl;
};

#endif

class Mutex
{
public:
	Mutex() {
#if ENTRY_PLATFORM_WINDOWS
		handle = new CRITICAL_SECTION;
		InitializeCriticalSection((CRITICAL_SECTION*)handle);
#else
		handle = new pthread_mutex_t;
		pthread_mutex_t* mutex = (pthread_mutex_t*)handle;
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(mutex, &attr);
#endif
	}

	~Mutex() {
#if ENTRY_PLATFORM_WINDOWS
		CRITICAL_SECTION* cs = (CRITICAL_SECTION*)handle;
		DeleteCriticalSection(cs);
		delete cs;
		handle = NULL;
#else
		pthread_mutex_t* mutex = (pthread_mutex_t*)handle;
		pthread_mutex_destroy(mutex);
		delete mutex;
		handle = 0;
#endif
	}

	void Acquire() {
#if ENTRY_PLATFORM_WINDOWS
		EnterCriticalSection((CRITICAL_SECTION*)handle);
#else
		pthread_mutex_lock((pthread_mutex_t*)handle);
#endif
	}

	bool TryAcquire() {
#if ENTRY_PLATFORM_WINDOWS
		return TryEnterCriticalSection((CRITICAL_SECTION*)handle) != FALSE;
#else
		return pthread_mutex_trylock((pthread_mutex_t*)handle) == 0;
#endif
	}

	void Release() {
#if ENTRY_PLATFORM_WINDOWS
		LeaveCriticalSection((CRITICAL_SECTION*)handle);
#else
		pthread_mutex_unlock((pthread_mutex_t*)handle);
#endif
	}

private:
	void* handle;
};

class ScopedLock {
public:
	ScopedLock(Mutex& _mutex) :
		mutex(_mutex)
	{
		mutex.Acquire();
	}
	~ScopedLock()
	{
		mutex.Release();
	}
private:
	ScopedLock(const ScopedLock& _rhs);
	ScopedLock& operator =(const ScopedLock& _rhs);
	Mutex& mutex;
};

class Thread {
public:
#ifdef _WIN32

	static DWORD WINAPI ThreadFunctionStatic(void* data)
	{
		Thread* thread = static_cast<Thread*>(data);
		thread->ThreadFunction();
		return 0;
	}

#else

	static void* ThreadFunctionStatic(void* data)
	{
		Thread* thread = static_cast<Thread*>(data);
		thread->ThreadFunction();
		pthread_exit((void*)0);
		return 0;
	}

#endif

	Thread() :
		thandle(NULL),
		shouldRun(false)
	{}

	~Thread()
	{
		Stop();
	}

	virtual void ThreadFunction() = 0;

	bool Run()
	{
		if (thandle)
			return false;

		shouldRun = true;
#if ENTRY_PLATFORM_WINDOWS
		thandle = CreateThread(nullptr, 0, ThreadFunctionStatic, this, 0, nullptr);
#else
		thandle = new pthread_t;
		pthread_attr_t type;
		pthread_attr_init(&type);
		pthread_attr_setdetachstate(&type, PTHREAD_CREATE_JOINABLE);
		pthread_create((pthread_t*)thandle, &type, ThreadFunctionStatic, this);
#endif		
		return thandle != NULL;
	}

	void Stop() {
		if (!thandle)
			return;
		shouldRun = false;
#if ENTRY_PLATFORM_WINDOWS
		WaitForSingleObject((HANDLE)thandle, INFINITE);
		CloseHandle((HANDLE)thandle);
#else
		pthread_t* thread = (pthread_t*)thandle;
		if (thread)
			pthread_join(*thread, 0);
		delete thread;
#endif	
		thandle = NULL;
	}

protected:
	void* thandle;
	volatile bool shouldRun;
};


//////////////////////////////////////////////////////////////////////
// Start: Logging

static Mutex logMutex;

void Log(const char* _info){
	ScopedLock lock (logMutex);
	if(!(gFlags & ENTRY_SILENT))
		std::cout << _info << std::endl;
}

// End: Logging Functions
//////////////////////////////////////////////////////////////////////


#ifndef __APPLE__
static const unsigned BUFFERSIZE = 4096;
#endif

class FileWatcher : public Thread
{
public:
	FileWatcher() :
		timer(Timer()),
		timerRem(Timer()),
		removed(false),
		changed(false),
		delay(1.0f)
	{
#if ENTRY_PLATFORM_LINUX
		watchHandle = -1;
		watchHandle = inotify_init();
#elif ENTRY_PLATFORM_MACOS
		_queueFD = -1;
		_dirFD = -1;
		_stopped = false;
#endif
	}

	~FileWatcher() {
		StopWatching();
#if ENTRY_PLATFORM_LINUX
		close(watchHandle);
#elif ENTRY_PLATFORM_MACOS
		close(_dirFD);
		close(_queueFD);
#endif
	}

	bool StartWatching(const std::string& _path) {
		StopWatching();
#if ENTRY_PLATFORM_WINDOWS
		dirHandle = (void*)CreateFileW(
			utf8toUtf16(RemoveTrailingSlash(_path)).c_str(),
			FILE_LIST_DIRECTORY,
			FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
			NULL);

		if (dirHandle != INVALID_HANDLE_VALUE){
			path = StringAppend(_path);
			Run();
			return true;
		}
		else {
			return false;
		}
#elif ENTRY_PLATFORM_LINUX
		int flags = IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVE;
		int Handle = inotify_add_watch(watchHandle, RemoveTrailingSlash(_path).c_str(), flags);
		if (Handle == -1) {
			printf("Failed inotify: %s err:%d\n", _path.c_str(), errno);
			return false;
		}

		dirHandle[Handle] = "";
		path = StringAppend(_path);
		Run();
		return true;

#elif ENTRY_PLATFORM_MACOS
		_dirFD = open(_path.c_str(), O_EVTONLY);
		if(_dirFD < 0){
			printf("Unable to open directory \n");
			return false;
		}
		_queueFD = kqueue();
		if (_queueFD < 0){
			close(_dirFD);
			printf("Cannot create kqueue \n");
			return false;
			//("Cannot create kqueue", errno);
		}
		path = StringAppend(_path);
		_stopped = false;
		Run();
		return true;
#endif
	}

	void StopWatching() {
		if (thandle)
		{
			shouldRun = false;

#if ENTRY_PLATFORM_WINDOWS
		std::string dummyFileName = path + "dummy.tmp";
		std::fstream fs;
		fs.open(dummyFileName, std::ios::out);
		fs.close();
		FileDelete(dummyFileName);
#endif

#if ENTRY_PLATFORM_MACOS
		Stop();
#endif	

#if ENTRY_PLATFORM_WINDOWS
			CloseHandle((HANDLE)dirHandle);
#elif ENTRY_PLATFORM_LINUX
			for (std::map<int, std::string>::iterator i = dirHandle.begin(); i != dirHandle.end(); ++i)
				inotify_rm_watch(watchHandle, i->first);
			dirHandle.clear();
#elif ENTRY_PLATFORM_MACOS
			_stopped = true;
			//CloseFileWatcher(watcher);
#endif	

#if !ENTRY_PLATFORM_MACOS
			Stop();
#endif	
			path.clear();
		}
	}

	std::string GetPath() {
		return path;
	}

	void SetDelay(float _interval) {
		delay = std::max(_interval, 0.0f);
	}

	void ThreadFunction() {
#if ENTRY_PLATFORM_WINDOWS
		unsigned char buffer[BUFFERSIZE];
		DWORD bytesFilled = 0;
		while (shouldRun) {
			if (ReadDirectoryChangesW((HANDLE)dirHandle,
				buffer,
				BUFFERSIZE,
				false,
				FILE_NOTIFY_CHANGE_FILE_NAME |
				FILE_NOTIFY_CHANGE_DIR_NAME |
				FILE_NOTIFY_CHANGE_CREATION |
				FILE_NOTIFY_CHANGE_LAST_WRITE,
				&bytesFilled,
				nullptr,
				nullptr))
			{
				unsigned offset = 0;

				while (offset < bytesFilled)
				{
					const FILE_NOTIFY_INFORMATION* record = (FILE_NOTIFY_INFORMATION*)&buffer[offset];
					
					std::wstring fileName(record->FileName, (record->FileNameLength)/2);
					std::wstring tmp = fileName;
					std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
					if(gLibName == converter.to_bytes(tmp)){
						if(record->Action & (FILE_ACTION_ADDED | FILE_ACTION_MODIFIED))
							SetChanged();
						else if( record->Action & FILE_ACTION_REMOVED)
							SetRemoved();
						//else
						//	printf("unhandled action %d\n", record->Action);
					}
					if (!record->NextEntryOffset)
						break;
					else
						offset += record->NextEntryOffset;
				}
			}
		}
#elif ENTRY_PLATFORM_LINUX
		unsigned char buffer[BUFFERSIZE];
		while (shouldRun) {
			int i = 0;
			int length = (int)read(watchHandle, buffer, sizeof(buffer));

			if (length < 0) return;

			while (i < length) {
				inotify_event* ev = (inotify_event*)&buffer[i];

				if (ev->len > 0) {
					if (ev->mask & IN_MODIFY || ev->mask & IN_MOVE) {
						AddChange(dirHandle[ev->wd] + ev->name);
					}
				}
				i += sizeof(inotify_event) + ev->len;
			}
		}
#elif ENTRY_PLATFORM_MACOS
		while (!_stopped)
		{
			struct timespec timeout;
			timeout.tv_sec = 0;
			timeout.tv_nsec = 200000000;
			unsigned eventFilter = NOTE_WRITE;
			struct kevent event;
			struct kevent eventData;
			EV_SET(&event, _dirFD, EVFILT_VNODE, EV_ADD | EV_CLEAR, eventFilter, 0, 0);
			int nEvents = kevent(_queueFD, &event, 1, &eventData, 1, &timeout);
			if (nEvents < 0 || eventData.flags == EV_ERROR)
			{
	/*			try
				{
					FileImpl::handleLastErrorImpl(owner().directory().path());
				}
				catch (Poco::Exception& exc)
				{
					owner().scanError(&owner(), exc);
				}*/
			}
			else if (nEvents > 0 || true)
			{
				ItemInfoMap newEntries;
				ItemInfo::scan(newEntries, path);
	//			compare(entries, newEntries);
	//			std::swap(entries, newEntries);
	//			lastScan.update();
			}
		}
#endif
	}

	bool IsChanged(){
		ScopedLock lock(mutex);
		unsigned delayMsec = (unsigned)(delay * 1000.0f);
		if(changed && timer.GetMSec(false) >= delayMsec){
			changed = false;
			return true;
		}
		return false;
	}

	bool IsRemoved(){
		ScopedLock lock(mutex);
		unsigned delayMsec = (unsigned)(delay * 1000.0f);
		if(removed && timerRem.GetMSec(false) >= delayMsec){
			removed = false;
			return true;
		}
		return false;
	}

private:
	class Timer
	{
	public:
		Timer() {
			Reset();
		}

		unsigned GetMSec(bool reset){
			unsigned currentTime = Tick();
			unsigned elapsedTime = currentTime - startTime_;
			if (reset)
				startTime_ = currentTime;

			return elapsedTime;
		}

		void Reset() {
			startTime_ = Tick();
		}

		static unsigned Tick()
		{
#ifdef _WIN32
			return (unsigned)timeGetTime();
#elif __EMSCRIPTEN__
			return (unsigned)emscripten_get_now();
#else
			struct timeval time;
			gettimeofday(&time, NULL);
			return (unsigned)(time.tv_sec * 1000 + time.tv_usec / 1000);
#endif
		}

	private:
		unsigned startTime_;
	};

	void SetChanged(){
		ScopedLock lock(mutex);		
		changed = true;
		timer.Reset();
	}

	void SetRemoved(){
		ScopedLock lock(mutex);
		removed = true;
		timerRem.Reset();
	}

	std::string path;
	Mutex mutex;
	Timer timer;
	Timer timerRem;
	bool removed;
	bool changed;
#if ENTRY_PLATFORM_WINDOWS
	void * dirHandle;
#elif ENTRY_PLATFORM_LINUX
	std::map<int, std::string> dirHandle;
	int watchHandle;
#elif ENTRY_PLATFORM_MACOS
	class ItemInfo;
	typedef std::map< std::string, ItemInfo> ItemInfoMap;

	class ItemInfo
	{
	public:
		ItemInfo() :
			size(0)
			{}

		ItemInfo(const ItemInfo& _rhs) :
			path(_rhs.path),
			size(_rhs.size),
			lastModified(_rhs.lastModified)
		{}

		ItemInfo(const std::string& _path, uint64_t _lastModified, uint64_t _size) :
			path(_path),
			size(_size),
			lastModified(_lastModified)
		{}

		static uint64_t fileLastModified(const char* _path){
			struct stat st;
			if (stat(_path, &st) == 0)
				return st.st_mtime;
			else
				return 0;
		}

		static void scan(ItemInfoMap& _entries, const std::string& _path){
			DirIter it(_path);
			while(it.HasNext()){
				const std::string res = _path + it.GetName();
				std::cout << FileSize(res.c_str()) <<std::endl;
				if(FileExists(res.c_str())){
					_entries[it.GetName()] = ItemInfo(	it.GetName(), 
														fileLastModified(res.c_str()), 
														FileSize(res.c_str()));//ItemInfo(*it);
				}
				it.Next();	
			}
		}

		static void compare(){

		}

	private:
		std::string path;
		uint64_t size;
		uint64_t lastModified;
	};

	int _queueFD;
	int _dirFD;
	bool _stopped;	
#endif
	float delay;
};

const char* Entry_GetPath() {
	static std::string appDirectory;

#if ENTRY_PLATFORM_WINDOWS
	if (!appDirectory.empty()) return _strdup(appDirectory.c_str());

	wchar_t path[MAX_PATH];
	path[0] = 0;
	GetModuleFileNameW(0, path, MAX_PATH);
	std::wstring tmp = path;
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	appDirectory = converter.to_bytes(tmp);
	return _strdup(appDirectory.c_str());
#elif ENTRY_PLATFORM_LINUX
	if (!appDirectory.empty()) return strdup(appDirectory.c_str());

	char path[PATH_MAX];
	memset(path, 0, PATH_MAX);
	std::string link;
	link.append("/proc/self/exe");
	readlink(link.c_str(), path, PATH_MAX);
	appDirectory = path;
	return strdup(appDirectory.c_str());
#else // ENTRY_PLATFORM_MACOS
	char path[1024];
	uint32_t size = sizeof(path);
	if (_NSGetExecutablePath(path, &size) != 0){
	    printf("buffer too small; need size %u\n", size);
	    return "";
	}
	std::string res = path;
	return res.c_str();
#endif
}

std::string Entry_GetDir() {
	const std::string res = StringReplace(Entry_GetPath(), "\\", "/");
	return res.substr(0, res.find_last_of('/')+1);
}

const char* GetAbsPath(const char* _path) {
#if ENTRY_PLATFORM_WINDOWS
	LPWSTR pPathPtr;
	WCHAR szwReturnedPath[_MAX_DIR + 1];
	GetFullPathNameW(utf8toUtf16(_path).c_str(), 256, szwReturnedPath, &pPathPtr);
	std::wstring tmp = pPathPtr;
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	return _strdup(converter.to_bytes(tmp).c_str());
#else
	char resolved_path[PATH_MAX];
	char* resultError = 0;
	resultError = realpath(_path, resolved_path);
	if (!resultError) {

	}
	return strdup(resolved_path);
#endif
}

const char* GetCurrentPath() {
#if ENTRY_PLATFORM_WINDOWS
	wchar_t result_path[MAX_PATH];
	result_path[0] = 0;

	DWORD dwRet = GetCurrentDirectoryW(MAX_PATH, result_path);
	if (dwRet == 0)
	{
		// Handle an error condition.
		//printf("GetFullPathName failed (%d)\n", GetLastError());
	}
	wcscat_s(result_path, L"\\");

	/* Replace '\\' with / */
	int i = 0;
	while (result_path[i])
	{
		if (result_path[i] == L'\\')
			result_path[i] = L'/';
		i++;
	}
	std::wstring tmp = result_path;
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	return _strdup(converter.to_bytes(tmp).c_str());
#else
	char result_path[PATH_MAX];
	result_path[0] = 0;
	char* resultError = 0;
	resultError = getcwd(result_path, PATH_MAX);
	if (!resultError) {

	}
	return strdup(result_path);
#endif
}

const char* GetTmpDir() {
#if ENTRY_PLATFORM_WINDOWS
	TCHAR lpTempPathBuffer[MAX_PATH];
	GetTempPathW(MAX_PATH, lpTempPathBuffer);
	std::wstring tmp = lpTempPathBuffer;
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	return _strdup(converter.to_bytes(tmp).c_str());
#else
#	if defined(P_tmpdir)
		const std::string res = StringAppend(P_tmpdir);
		return strdup(res.c_str());
#	else
	char const *folder = getenv("TMPDIR");
	if (folder == '\0')
		return strdup("/tmp/");
	else {
		const std::string res = StringAppend(folder);
		return strdup(res.c_str());
	}
#	endif
#endif
}

const char* GetDefaultPrefix() {
#if ENTRY_PLATFORM_ANDROID || ENTRY_PLATFORM_LINUX || ENTRY_PLATFORM_MACOS
	return "lib";
#else
	return "";
#endif
}

const char* GetDefaultSuffix() {
#if ENTRY_PLATFORM_WINDOWS
	return ".dll";
#elif ENTRY_PLATFORM_MACOS
	return ".dylib";
#else
	return ".so";
#endif
}

static FileWatcher fileWatcher;

int Entry_Attach(const char* _dir, const char* _name, const char * _prefix, const char * _suffix)
{
	// If empty use platform specific.
	const std::string prefix = (_prefix[0] == '?') ? GetDefaultPrefix() : _prefix;
	const std::string suffix = (_suffix[0] == '?') ? GetDefaultSuffix() : _suffix;
	std::string dir = StringAppend((_dir[0] == '\0') ? Entry_GetDir() : _dir);
	gLibName = prefix + std::string(_name) + suffix;
	DestroyLib(gLibrary);

	// Check if we can find the gLibrary at all.
	std::string path = dir + gLibName;
	if (!FileExists(path.c_str())) {
		dir = GetCurrentPath();
		path = dir + prefix + std::string(_name) + suffix;
		if (!FileExists(path.c_str())) {
			return 1;
		}
	}

	// Move lib.
	const std::string tmpLib = GetTmpDir() + gLibName;
	FileDelete(tmpLib);
	FileCopy(path, tmpLib);
	
	gNotifyEnabled = fileWatcher.StartWatching(dir);

	gLibrary = LoadLib(gLibrary, tmpLib);
		
	Log((std::string("Attaching itself to: ")+gLibName+std::string(" at ")+dir).c_str());

	if(!gLibrary)
		Log((std::string("Unsuccesfull attachment...")).c_str());

	Init = (PTR_Init)LoadFunction(gLibrary, "Init");

	if (Init){ if (Init()) return 2; };

	Reload = (PTR_Reload)LoadFunction(gLibrary, "Reload");
	Update = (PTR_Update)LoadFunction(gLibrary, "Update");

	if (Reload){ if (Reload()>0) return 2;};

	return (gLibrary == 0);
}

int Entry_Run(int _flags)
{
	gFlags = _flags;
	if (gLibrary != 0) {
		// Check if gLibrary was changed reload.
		if (gNotifyEnabled && fileWatcher.IsChanged()) {
			DestroyLib(gLibrary);
			const std::string tmpLib = GetTmpDir() + gLibName;
			FileDelete(tmpLib);
			FileCopy(fileWatcher.GetPath()+gLibName, tmpLib);

			gLibrary = LoadLib(gLibrary, tmpLib);
		
			Reload = (PTR_Reload)LoadFunction(gLibrary, "Reload");
			Update = (PTR_Update)LoadFunction(gLibrary, "Update");

			if (Reload) return !Reload();
		}

		if(Update) return !Update();

		// User wants the gLibrary to quit.
		if (!Reload && !Update) return 0;
		return 1;
	}
	return 0;
}