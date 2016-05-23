// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

namespace global
{
   bool initialized;
   HMODULE thisModule;
   UINT thisModuleBase;
   UINT thisModuleSize;
}

void Initialize(HMODULE hModule)
{
   global::initialized = true;
   int temp = sizeof(LogRecord);
   global::thisModule = hModule;
   auto rec = global::ModuleManager.GetThisModuleBoundaries(hModule);

   global::thisModuleBase = rec.ModuleInfo.Base;
   global::thisModuleSize = rec.ModuleInfo.Size;

   global::FileLogManager.Open(Utility::GetFileInThisModulePath(LOG_DATA_FILE_NAME));
   global::ModuleManager.TrackInitialModules();
   hooks::SetAllHooks();
}

_declspec(dllexport) DWORD Finish(char* dummy)
{
   if (global::initialized)
   {
      global::HookManager.RemoveHooks();
      global::FileLogManager.Close();
      global::initialized = false;
   }
   return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved
                      )
{
   switch (ul_reason_for_call)
   {
   case DLL_PROCESS_ATTACH:
      global::initialized = false;
      Initialize(hModule);
      break;
   case DLL_THREAD_ATTACH:
      break;
   case DLL_THREAD_DETACH:
      break;
   case DLL_PROCESS_DETACH:
      Finish(NULL);
      break;
   }
   return TRUE;
}

