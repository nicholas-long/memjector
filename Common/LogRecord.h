#pragma once

#include <Windows.h>

#pragma pack(push, 1)
struct LogRecord
{
   enum Type
   {
      LOAD_MODULE,
      UNLOAD_MODULE,
      ALLOC,
      FREE
   };

   struct _AllocInfo
   {
      LPVOID Memory; // for ALLOC and FREE
      UINT BlockSize;
      UINT Callstack[CALLSTACK_SIZE]; // for ALLOC
   };

   struct _ModuleInfo
   {
      HMODULE ModuleHandle; // for LOAD_MODULE and UNLOAD_MODULE
      UINT Base, Size; // for LOAD_MODULE
      char FilePath[FILEPATH_SIZE]; // for LOAD_MODULE and UNLOAD_MODULE
   };

   Type RecordType;

   union
   {
      _AllocInfo AllocInfo;
      _ModuleInfo ModuleInfo;
   };
};
#pragma pack(pop)