#pragma once

struct ModuleInformation
{
   HMODULE hmod;
   UINT base, size;
   std::string fileFullPath;
   std::string fileName;
};

class _ModuleDetailManager
{
public:
   _ModuleDetailManager(void);
   ~_ModuleDetailManager(void);

   void TrackInitialModules(void);
   void InformLoadModule(HMODULE hmod);
   void InformUnloadModule(HMODULE hmod);

   LogRecord GetThisModuleBoundaries(HMODULE hmod);

private:
   bool GetModuleInfo(HMODULE hmod, LogRecord& result);
};

