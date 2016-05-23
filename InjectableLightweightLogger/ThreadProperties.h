#pragma once

#include "stdafx.h"

struct ThreadProperties
{
   ThreadProperties() : ThreadId(0), HookingHeapAlloc(false), HookingHeapFree(false), CurrentlyLogging(false)
   {};

   ThreadProperties(DWORD _threadId) : ThreadId(_threadId), HookingHeapAlloc(false), HookingHeapFree(false), CurrentlyLogging(false)
   {};

   DWORD ThreadId;

   bool 
      HookingHeapAlloc,
      HookingHeapFree,
      CurrentlyLogging;
};