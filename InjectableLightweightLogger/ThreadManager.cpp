#include "stdafx.h"
#include "ThreadManager.h"


namespace global
{
   boost::mutex ThreadManagerMutex;
}

_ThreadManager::_ThreadManager(void)
{
}


_ThreadManager::~_ThreadManager(void)
{
}

ThreadProperties& _ThreadManager::GetThreadProperties()
{
   DWORD id = GetCurrentThreadId();
   int index;
   if ((index = FindThreadProperty(id) < 0))
      return properties[AddThreadProperty(id)];
   else
      return properties[index];
}

int _ThreadManager::AddThreadProperty(DWORD id)
{
   boost::mutex::scoped_lock lock(global::ThreadManagerMutex);

   int result = propertyCount++;
   properties[result] = ThreadProperties(id);
   return result;
}

int _ThreadManager::FindThreadProperty(DWORD id)
{
   for (int n = 0; n < propertyCount; n++)
   {
      if (properties[n].ThreadId == id)
         return n;
   }
   return -1;
}