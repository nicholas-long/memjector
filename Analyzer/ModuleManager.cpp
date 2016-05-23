#include "stdafx.h"
#include "ModuleManager.h"
#include <Psapi.h>
#include <boost/algorithm/string.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/filesystem.hpp>

using std::vector;
using std::map;
using std::set;
using std::string;
using std::wstring;
using std::stringstream;

struct ModuleInformation;

boost::mutex accessMutex;
map<HMODULE, ModuleInformation> gmapHmodToModuleInfo;
map<string, ModuleInformation> gmapNameToModuleInfo;
set<string> moduleLoadedNames;
vector<ModuleInformation> sortedModuleList;

typedef boost::recursive_mutex::scoped_lock ScopedRecursiveLock;
boost::recursive_mutex moduleAccessMutex;


inline string getStringFromW(wstring temp)
{
	return string(temp.begin(), temp.end());
}

inline string getStringFromArr(const wchar_t * a)
{
	return getStringFromW(wstring(a));
}

//bool getModuleInfo(HMODULE hmod, ModuleInformation& result)
//{
//	result.hmod = hmod;
//
//	const size_t fileNameLength = 1000;
//
//   
//	//HANDLE hProcess = GetCurrentProcess();
//   HANDLE hProcess = global::hProcess;
//
//	TCHAR buffer[fileNameLength];
//
//	GetModuleFileName(hmod, buffer, fileNameLength);
//	result.fileFullPath = getStringFromArr(buffer);
//
//	GetModuleBaseName(hProcess, hmod, buffer, fileNameLength);
//	result.fileName = getStringFromArr(buffer);
//	boost::to_lower(result.fileName);
//	
//	MODULEINFO modinfo;
//	if (!GetModuleInformation(hProcess, hmod, &modinfo, sizeof(MODULEINFO))) return false;
//
//	result.base = (UINT)modinfo.lpBaseOfDll;
//	result.size = modinfo.SizeOfImage;
//
//	return true;
//}

bool getModuleInfo(LogRecord& rec, ModuleInformation& result)
{
   result.fileFullPath = rec.ModuleInfo.FilePath;
   boost::filesystem::path p(rec.ModuleInfo.FilePath);
   result.fileName = p.filename().string();
   result.hmod = rec.ModuleInfo.ModuleHandle;
   result.base = rec.ModuleInfo.Base;
   result.size = rec.ModuleInfo.Size;
   return true;
}

namespace global
{
	ModuleManager moduleManager;
}

ModuleManager::ModuleManager(void)
{
}


ModuleManager::~ModuleManager(void)
{
}


void ModuleManager::TrackInitialModules(void)
{
	////ScopedRecursiveLock l(moduleAccessMutex);
	//HANDLE hProcess = GetCurrentProcess();
	//DWORD count = 0;
	//HMODULE hMods[1024];
	//EnumProcessModules(hProcess, hMods, sizeof(hMods), &count);
	//for (DWORD n = 0; n < (count / 4); n++) //Count returned is in bytes.
	//{
	//	InformLoadModule(hMods[n]);
	//}
}


void ModuleManager::InformLoadModule(HMODULE hmod, LogRecord rec)
{
	//ScopedRecursiveLock l(moduleAccessMutex);
	ModuleInformation modinfo;
	//bool ok = getModuleInfo(hmod, modinfo);
   getModuleInfo(rec, modinfo);
	if (true)
	{
		gmapHmodToModuleInfo[hmod] = modinfo;
		sortedModuleList.push_back(modinfo);
		std::sort(sortedModuleList.begin(), sortedModuleList.end(), [](const ModuleInformation& a, const ModuleInformation& b){
			return a.base < b.base;
		});
	}
	global::symbolManager.InformLoadModule(modinfo);
	//gmapNameToModuleInfo[modinfo.fileName] = modinfo;
	//moduleLoadedNames.insert(modinfo.fileFullPath);
}


void ModuleManager::InformUnloadModule(HMODULE hmod)
{
	ScopedRecursiveLock l(moduleAccessMutex);
	ModuleInformation temp = gmapHmodToModuleInfo[hmod];
	gmapHmodToModuleInfo.erase(hmod);
	vector<ModuleInformation> copied;
	std::remove_copy_if(sortedModuleList.begin(), sortedModuleList.end(), std::back_inserter(copied), [hmod](ModuleInformation& m){
		return m.hmod == hmod;
	});
	sortedModuleList = copied;
	global::symbolManager.InformUnloadModule(temp);
}

ModuleInformation& findByAddress(UINT address)
{
	auto& l = sortedModuleList;
	for (auto it = l.begin(); it != l.end(); it++)
	{
		if (it->base <= address && address <= (it->base + it->size)) return *it;
	}
	//size_t index = (l.size() / 2) + 1, drift = index;
	//while (true)
	//{
	//	if (l[index].base <= address && address <= l[index].base + l[index].size)
	//	{
	//		return l[index];
	//	}
	//	else if (address < l[index].base)
	//	{
	//		index = max(index - drift, 0);
	//	}
	//	else
	//	{
	//		index = min(index + drift, l.size());;
	//	}
	//	if (drift == 0) throw std::string("UNKNOWN");
	//	drift /= 2;
	//}
	throw std::string("UNKNOWN");
}

std::string ModuleManager::GetModuleNameFromAddress(UINT address)
{
	ScopedRecursiveLock l(moduleAccessMutex);
	stringstream s;
	try
	{
		ModuleInformation& modinfo = findByAddress(address);
		s << modinfo.fileName << "+" << std::hex << (address - modinfo.base);
	}
	catch(std::string& e)
	{
		s << e << "!" << std::hex << address;
	}
	return s.str();
}

ModuleInformation& ModuleManager::GetModuleInfoFromAddress(UINT address)
{
	ScopedRecursiveLock l(moduleAccessMutex);
	return findByAddress(address);
}