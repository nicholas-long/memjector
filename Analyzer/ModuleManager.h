#pragma once

#include <string>

struct ModuleInformation
{
	HMODULE hmod;
	std::string fileFullPath;
	std::string fileName;
	UINT base, size;
};

class ModuleManager
{
public:
	ModuleManager(void);
	~ModuleManager(void);
	void TrackInitialModules(void);
	void InformLoadModule(HMODULE hmod, LogRecord rec);
	void InformUnloadModule(HMODULE hmod);
	std::string GetModuleNameFromAddress(UINT address);
	ModuleInformation& GetModuleInfoFromAddress(UINT address);
};

