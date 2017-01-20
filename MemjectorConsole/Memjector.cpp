#include "stdafx.h"
#include "../Common/Constants.h"

#include <Windows.h>
#include <Psapi.h>

#include <iostream>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

using boost::filesystem::path;
using std::string;
using namespace std;

const char * DLL_FILENAME_LOGGER = "InjectableLightweightLogger.dll";
//const char * DLL_FILENAME_STOP = "FinishMemoryLogging.dll"; // removed this DLL which terminates memory logging in favor of calling FreeLibrary and working around the hook
const char * EXE_FILENAME_ANALYZE = "Analyzer.exe";

// thanks http://stackoverflow.com/questions/10930353/injecting-c-dll/10981735
bool Inject(DWORD pId, const char *dllName)
{
   HANDLE h = OpenProcess(PROCESS_ALL_ACCESS, false, pId);
   if(h)
   {
      LPVOID LoadLibAddr = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
      LPVOID dereercomp = VirtualAllocEx(h, NULL, strlen(dllName), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
      WriteProcessMemory(h, dereercomp, dllName, strlen(dllName), NULL);
      HANDLE asdc = CreateRemoteThread(h, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibAddr, dereercomp, 0, NULL);
      WaitForSingleObject(asdc, INFINITE);
      VirtualFreeEx(h, dereercomp, strlen(dllName), MEM_RELEASE);
      CloseHandle(asdc);
      CloseHandle(h);
      return true;
   }
   return false;
}

HMODULE FindHModule(HANDLE proc, const char * dllName)
{
   string dll = dllName;
   boost::to_lower(dll);

   DWORD count = 0;
   HMODULE hMods[1024];
   EnumProcessModules(proc, hMods, sizeof(hMods), &count);

   for (DWORD i = 0; i < count; i++)
   {
      HMODULE m = hMods[i];
      char buf[1024];
      GetModuleFileNameExA(proc, m, buf, 1024);
      string thisModule(buf);
      
      path p(thisModule);
      thisModule = p.filename().string();

      boost::to_lower(thisModule);

      if (thisModule == dll)
         return m;
   }
   
   return 0;
}

bool Uninject(DWORD pId, const char *dllName)
{
   HANDLE h = OpenProcess(PROCESS_ALL_ACCESS, false, pId);
   if(h)
   {
      HMODULE dll = FindHModule(h, dllName);
      if (!dll) return false;

      LPVOID LoadLibAddr = (LPVOID)((BYTE*)GetProcAddress(GetModuleHandleA("kernel32.dll"), "FreeLibrary") + 2); // add 2 to bypass the hook we know is there

      HANDLE asdc = CreateRemoteThread(h, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibAddr, dll, 0, NULL);
      WaitForSingleObject(asdc, INFINITE);
      CloseHandle(asdc);
      CloseHandle(h);
      return true;
   }
   return false;
}

std::string GetFileInThisModulePath(std::string fileName)
{
   char fileNameBuf[1000];
   GetModuleFileNameA(0, fileNameBuf, 1000);
   path p(fileNameBuf);
   return p.parent_path().append(fileName).string();
}

int _tmain(int argc, _TCHAR* argv[])
{
   DWORD processId;

   if (argc > 1)
   {
      processId = boost::lexical_cast<DWORD>(std::wstring(argv[1]));
   }
   else
   {
      //cout << "Enter process ID, or 0 to find a process: ";
      cout << "Enter process ID: ";
      cin >> processId;
      //if (processId == 0)
      //processId = HelpFindProcessId();
   }

   if (!Inject(processId, GetFileInThisModulePath(DLL_FILENAME_LOGGER).c_str()))
   {
      cout << "An error occurred while trying to attach to the process.  " << std::hex << GetLastError() << endl;
      cout << "Perhaps this process does not have adequate permissions to attach.  You could try rerunning as admin." << endl;
      cin.get();
      return 1;
   }

   cout << "Memory logging has begun.  Press enter when you are ready to stop logging." << endl;
   cin.get();
   cin.get();


   Uninject(processId, DLL_FILENAME_LOGGER);
   //if (!Inject(processId, GetFileInThisModulePath(DLL_FILENAME_STOP).c_str()))
   //{
   //   cout << "An unexpected error occurred." << endl;
   //   cout << "Attempting to proceed (analysis may fail)" << endl;
   //}
   //else
   //   Uninject(processId, DLL_FILENAME_LOGGER);

   //string keepGoing;
   //do
   //{
   //   stringstream ss;
   //   ss << "\"" << GetFileInThisModulePath(EXE_FILENAME_ANALYZE) << "\" " 
   //      << processId << " " << LOG_DATA_FILE_NAME;
   //   string cmdLine = ss.str();
   //   system(cmdLine.c_str());

   //   cout << "Analysis complete." << endl
   //      << "You may run analysis again with different configurations specified in the config.json file." << endl
   //      << "Would you like to do this?" << endl;

   //   cout << "Answer (Y/N): ";
   //   cin >> keepGoing;
   //}
   //while (keepGoing[0] == 'Y' || keepGoing[0] == 'y');

   return 0;
}

