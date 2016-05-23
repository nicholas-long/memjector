#include "stdafx.h"
#include "WinApiHooks.h"
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread.hpp>

namespace hooks
{
   namespace original
   {	//Original callable functions
      funcptrHeapAlloc HeapAlloc;
      funcptrHeapFree HeapFree;

      funcptrLoadLibraryW LoadLibraryW;
      funcptrLoadLibraryExW LoadLibraryExW;

      funcptrLoadLibraryA LoadLibraryA;
      funcptrLoadLibraryExA LoadLibraryExA;

      funcptrFreeLibrary FreeLibrary;
      funcptrFreeLibraryAndExitThread FreeLibraryAndExitThread;
   }


   //Function for reading stack boundaries from Mass (Mologie.de)

   typedef struct _teb {
      void *ExceptionList;
      UINT_PTR StackBase;
      UINT_PTR StackLimit;
   } teb_t;
   inline teb_t* getTeb() {
#ifdef _M_IX86
      return (teb_t*)__readfsdword(0x18);
#endif
#ifdef _M_AMD64
      return (teb_t*)__readgsqword(0x30);
#endif
   }

   //End stack boundaries


   //Hooking load and free library functions so we can track when DLLs get loaded and update list of modules

   //HMODULE WINAPI LoadLibrary(
   //	_In_  LPCTSTR lpFileName
   //	);
   HMODULE APIENTRY LoadLibraryA(LPCSTR lpFileName)
   {
      HMODULE m = original::LoadLibraryA(lpFileName);

      global::ModuleManager.InformLoadModule(m);

      return m;
   }

   //HMODULE WINAPI LoadLibrary(
   //	_In_  LPCTSTR lpFileName
   //	);
   HMODULE APIENTRY LoadLibraryW(LPCWSTR lpFileName)
   {
      HMODULE m = original::LoadLibraryW(lpFileName);
      global::ModuleManager.InformLoadModule(m);
      return m;
   }


   //HMODULE WINAPI LoadLibraryEx(
   //_In_        LPCTSTR lpFileName,
   //_Reserved_  HANDLE hFile,
   //_In_        DWORD dwFlags
   //);
   HMODULE APIENTRY LoadLibraryExA(LPCSTR lpFileName, HANDLE hFile,  DWORD dwFlags)
   {
      HMODULE m = original::LoadLibraryExA(lpFileName, hFile, dwFlags);
      global::ModuleManager.InformLoadModule(m);
      return m;
   }

   //HMODULE WINAPI LoadLibraryEx(
   //_In_        LPCTSTR lpFileName,
   //_Reserved_  HANDLE hFile,
   //_In_        DWORD dwFlags
   //);
   HMODULE APIENTRY LoadLibraryExW(LPCWSTR lpFileName, HANDLE hFile,  DWORD dwFlags)
   {
      HMODULE m = original::LoadLibraryExW(lpFileName, hFile, dwFlags);
      global::ModuleManager.InformLoadModule(m);
      return m;
   }

   //BOOL WINAPI FreeLibrary(
   //	_In_  HMODULE hModule
   //	);
   BOOL APIENTRY FreeLibrary(HMODULE hModule)
   {
      BOOL result = original::FreeLibrary(hModule);
      global::ModuleManager.InformUnloadModule(hModule);
      return result;
   }

   //VOID WINAPI FreeLibraryAndExitThread(
   //	_In_  HMODULE hModule,
   //	_In_  DWORD dwExitCode
   //	);
   VOID APIENTRY FreeLibraryAndExitThread(HMODULE hModule, DWORD dwExitCode)
   {
      global::ModuleManager.InformUnloadModule(hModule);
      return original::FreeLibraryAndExitThread(hModule, dwExitCode);
   }



   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Hooking HeapAlloc and HeapFree

   //Globals for assisting in thread safety and recursion

   //typedef std::vector<std::pair<UINT, UINT>> StackTraceFrames;
   typedef UINT StackTraceReturns[];

   inline void walkbackstack(StackTraceReturns stackTrace)
   {
      auto& boundaries = *getTeb();

      int position = 0;

      UINT_PTR retAddress = (UINT_PTR)_AddressOfReturnAddress();
      UINT_PTR ebpFrame = ((UINT_PTR)_AddressOfReturnAddress() - sizeof(UINT_PTR));
      UINT_PTR retCodeAddress = (*(PUINT_PTR)retAddress);
      for (; retCodeAddress != 0 && ebpFrame >= boundaries.StackLimit && ebpFrame <= boundaries.StackBase && position < CALLSTACK_SIZE ;)
      {
         stackTrace[position++] = retCodeAddress;
         ebpFrame = (UINT_PTR)(*(PUINT_PTR)ebpFrame);
         
         //UINT_PTR originalEbp = ebpFrame;
         //while (
         //   !((*(PUINT_PTR)ebpFrame) >= boundaries.StackLimit && (*(PUINT_PTR)ebpFrame) <= boundaries.StackBase || ebpFrame < originalEbp)
         //   && (ebpFrame >= boundaries.StackLimit && ebpFrame <= boundaries.StackBase)
         //   )
         //{
         //   ebpFrame += sizeof(UINT_PTR);
         //}

         retAddress = ebpFrame + sizeof(UINT_PTR);
         if (!(retAddress >= boundaries.StackLimit && retAddress <= boundaries.StackBase)) //"managed to native transition"...?
            break;
         retCodeAddress = (*(PUINT_PTR)retAddress);
      }
   }

   inline bool CalledFromThisDll(StackTraceReturns t)
   {
      auto i = 0;
      i++; //skip the most recent call
      for (; i < CALLSTACK_SIZE; i++)
      {
         try
         {
            if (t[i] == 0) return false;

            if (t[i] >= global::thisModuleBase && t[i] < global::thisModuleBase + global::thisModuleSize)
               return true;
         }
         catch(...)
         {}
      }
      return false;
   }

















   //LPVOID WINAPI HeapAlloc(
   //	_In_  HANDLE hHeap,
   //	_In_  DWORD dwFlags,
   //	_In_  SIZE_T dwBytes
   //	);
   LPVOID APIENTRY HeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes)
   {
      auto& threadProperties = global::ThreadManager.GetThreadProperties();

      //allow heapalloc to call itself recursively (with different flags)
      //also, don't track allocations from within stuff we are calling from here
      if (threadProperties.HookingHeapAlloc)
      {
         //if this is the same thread as the locked one, proceed without tracking (we are in the recursive call or tracking allocations in this DLL)
         return original::HeapAlloc(hHeap, dwFlags, dwBytes);
      }

      threadProperties.HookingHeapAlloc = true;

      //call the original
      LPVOID retVal = original::HeapAlloc(hHeap, dwFlags, dwBytes);
      {
         LogRecord rec;
         ZeroMemory(&rec, sizeof(LogRecord));

         rec.RecordType = LogRecord::ALLOC;
         rec.AllocInfo.Memory = retVal;
         rec.AllocInfo.BlockSize = (UINT)dwBytes;
         walkbackstack(rec.AllocInfo.Callstack);

         if (!CalledFromThisDll(rec.AllocInfo.Callstack))
            global::FileLogManager.Log(rec);
      }

      threadProperties.HookingHeapAlloc = false;

      return retVal;
   }

   //BOOL WINAPI HeapFree(
   //	_In_  HANDLE hHeap,
   //	_In_  DWORD dwFlags,
   //	_In_  LPVOID lpMem
   //	);
   BOOL APIENTRY HeapFree(HANDLE hHeap,DWORD dwFlags,LPVOID lpMem)
   {
      auto& threadProperties = global::ThreadManager.GetThreadProperties();

      //allow heapfree to call itself recursively (with different flags)
      //also, don't track allocations from within stuff we are calling from here

      if (threadProperties.HookingHeapFree) //&& heapFreeThreadId == threadId
         return original::HeapFree(hHeap, dwFlags, lpMem);

      threadProperties.HookingHeapFree = true;

      BOOL retVal = original::HeapFree(hHeap, dwFlags, lpMem);
      {
         LogRecord rec;
         ZeroMemory(&rec, sizeof(LogRecord));

         rec.RecordType = LogRecord::FREE;
         rec.AllocInfo.Memory = lpMem;

         global::FileLogManager.Log(rec);
      }

      threadProperties.HookingHeapFree = false;

      return retVal;
   }

   void SetAllHooks(void)
   {
      auto& hm = global::HookManager;

      HMODULE hmod = GetModuleHandleA("ntdll.dll");
      void 
         *allocateAddress = GetProcAddress(hmod, "RtlAllocateHeap"),
         *freeAddress = GetProcAddress(hmod, "RtlFreeHeap");

      hm.InsertHook(allocateAddress, hooks::HeapAlloc, (void**)&original::HeapAlloc);
      hm.InsertHook(freeAddress, hooks::HeapFree, (void**)&original::HeapFree);

      HMODULE kernelHmod = GetModuleHandleA("kernel32.dll");

      hm.InsertHook(GetProcAddress(kernelHmod, "LoadLibraryW"), hooks::LoadLibraryW, (void**)&original::LoadLibraryW);
      hm.InsertHook(GetProcAddress(kernelHmod, "LoadLibraryExW"), hooks::LoadLibraryExW, (void**)&original::LoadLibraryExW);

      hm.InsertHook(GetProcAddress(kernelHmod, "LoadLibraryA"), hooks::LoadLibraryA, (void**)&original::LoadLibraryA);
      hm.InsertHook(GetProcAddress(kernelHmod, "LoadLibraryExA"), hooks::LoadLibraryExA, (void**)&original::LoadLibraryExA);

      hm.InsertHook(GetProcAddress(kernelHmod, "FreeLibrary"), hooks::FreeLibrary, (void**)&original::FreeLibrary);
      hm.InsertHook(GetProcAddress(kernelHmod, "FreeLibraryAndExitThread"), hooks::FreeLibraryAndExitThread, (void**)&original::FreeLibraryAndExitThread);
   }
}