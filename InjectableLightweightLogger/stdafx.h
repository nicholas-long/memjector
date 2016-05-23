// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>



// TODO: reference additional headers your program requires here

#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <string>
#include <fstream>

#include "MemoryHookManager.h"
#include "WinApiHooks.h"
#include "../Common/Constants.h"
#include "ThreadManager.h"
#include "FileLogManager.h"
#include "ModuleDetailManager.h"
#include "Utility.h"

namespace global
{
   extern bool initialized;
   extern HMODULE thisModule;
   extern UINT thisModuleBase;
   extern UINT thisModuleSize;

   extern MemoryHooking HookManager;
   extern _ThreadManager ThreadManager;
   extern _FileLogManager FileLogManager;
   extern _ModuleDetailManager ModuleManager;
}