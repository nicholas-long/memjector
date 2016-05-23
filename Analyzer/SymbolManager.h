#pragma once
#include "ModuleManager.h"


class SymbolManager
{
public:
   SymbolManager(void);
   ~SymbolManager(void);
   bool GetFileNameAndLineForAddress(UINT addy, std::string& fileName, UINT& lineNumber);
   void InformLoadModule(ModuleInformation& modinfo);
   void InformUnloadModule(ModuleInformation& modinfo);
   void informSymbolPaths();
   void Initialize();
   void Cleanup();
};

