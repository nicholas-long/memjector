#pragma once

#include "stdafx.h"
#include "ThreadProperties.h"

class _ThreadManager
{
public:
   _ThreadManager(void);
   ~_ThreadManager(void);

   ThreadProperties& GetThreadProperties();

private:
   int AddThreadProperty(DWORD id);
   int FindThreadProperty(DWORD id);

   int propertyCount;
   ThreadProperties properties[1000]; // kind of lame


};

