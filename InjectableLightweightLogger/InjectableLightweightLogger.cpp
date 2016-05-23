// InjectableLightweightLogger.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

namespace global
{
   MemoryHooking HookManager;
   _ThreadManager ThreadManager;
   _FileLogManager FileLogManager;
   _ModuleDetailManager ModuleManager;
}