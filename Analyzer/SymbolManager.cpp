#include "stdafx.h"
#include "SymbolManager.h"
#include <DbgHelp.h>
#include <boost/thread/recursive_mutex.hpp>

#pragma comment(lib, "dbghelp.lib")

namespace global
{
	SymbolManager symbolManager;
}


using std::map;
using std::set;
using std::string;

typedef boost::recursive_mutex::scoped_lock ScopedRecursiveLock;
boost::recursive_mutex symbolTrackingMutex;

SymbolManager::SymbolManager(void)
{
	//ScopedRecursiveLock l(symbolTrackingMutex);
	//SymInitialize(global::hProcess, NULL, TRUE); //2nd parameter is path
}


SymbolManager::~SymbolManager(void)
{
	//ScopedRecursiveLock l(symbolTrackingMutex);
	//SymCleanup(global::hProcess);//cleanup
}

void SymbolManager::Initialize()
{
   SymInitialize(global::hProcess, NULL, TRUE); //2nd parameter is path
}

void SymbolManager::Cleanup()
{
   SymCleanup(global::hProcess);//cleanup
}

bool SymbolManager::GetFileNameAndLineForAddress(UINT addy, std::string& fileName, UINT& lineNumber)
{
	//ScopedRecursiveLock l(symbolTrackingMutex);
	_IMAGEHLP_LINE line;
	DWORD displacement = 0;
	if (SymGetLineFromAddr(global::hProcess, (DWORD)addy, &displacement, &line) != FALSE)
	{
		fileName = std::string(line.FileName);
		lineNumber = line.LineNumber;
		return true;
	}
	else return false;
}

void SymbolManager::InformLoadModule(ModuleInformation& modinfo)
{
	//ScopedRecursiveLock l(symbolTrackingMutex);
	//if (modinfo.hmod != global::thisModule)
	{
		SymLoadModule(global::hProcess, NULL, modinfo.fileFullPath.c_str(), modinfo.fileName.c_str(), modinfo.base, modinfo.size);
	}
}

void SymbolManager::InformUnloadModule(ModuleInformation& modinfo)
{
	//ScopedRecursiveLock l(symbolTrackingMutex);
	//if (modinfo.hmod != global::thisModule)

   //SymUnloadModule(global::hProcess, modinfo.base);
}

void SymbolManager::informSymbolPaths()
{
	//ScopedRecursiveLock l(symbolTrackingMutex);
	auto& dbgPaths = global::optionsManager.options.settings.debugSymbolPaths;
	if (dbgPaths.size() > 0)
	{
		string symbolPaths = Utility::JoinStrings(dbgPaths, ";");
		SymSetSearchPath(global::hProcess, symbolPaths.c_str());
	}
}