#include "stdafx.h"
#include "ModuleDetailManager.h"
#include <Psapi.h>
#include <boost/algorithm/string.hpp>

using namespace std;

_ModuleDetailManager::_ModuleDetailManager(void)
{
}

_ModuleDetailManager::~_ModuleDetailManager(void)
{
}

inline string getStringFromW(wstring temp)
{
	return string(temp.begin(), temp.end());
}

inline string getStringFromArr(const wchar_t * a)
{
	return getStringFromW(wstring(a));
}

void _ModuleDetailManager::TrackInitialModules(void)
{
	HANDLE hProcess = GetCurrentProcess();
	DWORD count = 0;
	HMODULE hMods[1024];
	EnumProcessModules(hProcess, hMods, sizeof(hMods), &count);
	for (DWORD n = 0; n < (count / 4); n++) //Count returned is in bytes.
	{
		InformLoadModule(hMods[n]);
	}
}

void _ModuleDetailManager::InformLoadModule(HMODULE hmod)
{
   LogRecord rec;
   rec.RecordType = LogRecord::LOAD_MODULE;

   ModuleInformation info;
   if (!GetModuleInfo(hmod, rec))
      return; // do something?

   global::FileLogManager.Log(rec);
}

void _ModuleDetailManager::InformUnloadModule(HMODULE hmod)
{
   LogRecord rec;
   rec.RecordType = LogRecord::UNLOAD_MODULE;
   rec.ModuleInfo.ModuleHandle = hmod;

   global::FileLogManager.Log(rec);
}

bool _ModuleDetailManager::GetModuleInfo(HMODULE hmod, LogRecord& result)
{
	result.ModuleInfo.ModuleHandle = hmod;

	const size_t fileNameLength = 1000;

	HANDLE hProcess = GetCurrentProcess();

	TCHAR buffer[fileNameLength];

	GetModuleFileName(hmod, buffer, fileNameLength);
	string fileFullPath = getStringFromArr(buffer);

   strcpy_s(result.ModuleInfo.FilePath, fileFullPath.c_str());

	//GetModuleBaseName(hProcess, hmod, buffer, fileNameLength);
	//result.fileName = getStringFromArr(buffer);
	//boost::to_lower(result.fileName);
	
	MODULEINFO modinfo;
	if (!GetModuleInformation(hProcess, hmod, &modinfo, sizeof(MODULEINFO))) return false;

	result.ModuleInfo.Base = (UINT)modinfo.lpBaseOfDll;
	result.ModuleInfo.Size = modinfo.SizeOfImage;

	return true;
}

LogRecord _ModuleDetailManager::GetThisModuleBoundaries(HMODULE hmod)
{
   LogRecord result;
   GetModuleInfo(hmod, result);
   return result;
}