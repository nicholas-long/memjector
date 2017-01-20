
namespace hooks
{
	//Function pointers
	//Alloc and Free
	typedef LPVOID (APIENTRY *funcptrHeapAlloc)(HANDLE,DWORD,SIZE_T);
	typedef BOOL (APIENTRY *funcptrHeapFree)(HANDLE,DWORD,LPVOID);

	//Loading DLLs
	typedef HMODULE(APIENTRY *funcptrLoadLibraryW)(LPCWSTR);
	typedef HMODULE(APIENTRY *funcptrLoadLibraryExW)(LPCWSTR,HANDLE,DWORD);
	typedef HMODULE(APIENTRY *funcptrLoadLibraryA)(LPCSTR);
	typedef HMODULE(APIENTRY *funcptrLoadLibraryExA)(LPCSTR,HANDLE,DWORD);

	//Unloading DLLs
	typedef BOOL(APIENTRY *funcptrFreeLibrary)(HMODULE);
	typedef VOID(APIENTRY *funcptrFreeLibraryAndExitThread)(HMODULE,DWORD);

	typedef FARPROC(APIENTRY *funcptrGetProcAddress)(HMODULE, LPCSTR);
	//FARPROC WINAPI GetProcAddressHook(
	//	_In_ HMODULE hModule,
	//	_In_ LPCSTR  lpProcName
	//)

	//HMODULE WINAPI LoadLibrary(
	//	_In_  LPCTSTR lpFileName
	//	);

	//HMODULE WINAPI LoadLibraryEx(
	//_In_        LPCTSTR lpFileName,
	//_Reserved_  HANDLE hFile,
	//_In_        DWORD dwFlags
	//);

	//BOOL WINAPI FreeLibrary(
	//	_In_  HMODULE hModule
	//	);

	//VOID WINAPI FreeLibraryAndExitThread(
	//	_In_  HMODULE hModule,
	//	_In_  DWORD dwExitCode
	//	);

	//namespace original
	//{	//Original callable functions
	//	extern funcptrHeapAlloc HeapAlloc;
	//	extern funcptrHeapFree HeapFree;

	//	extern funcptrLoadLibrary LoadLibrary;
	//	extern funcptrLoadLibraryEx LoadLibraryEx;
	//	extern funcptrFreeLibrary FreeLibrary;
	//	extern funcptrFreeLibraryAndExitThread FreeLibraryAndExitThread;
	//}

	//Hook versions
	LPVOID APIENTRY HeapAlloc(HANDLE,DWORD,SIZE_T);
	BOOL APIENTRY HeapFree(HANDLE,DWORD,LPVOID);
	void SetAllHooks(void);
}