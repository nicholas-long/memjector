// Analyzer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <boost/lexical_cast.hpp>

using namespace std;

namespace global
{
   DWORD processId;
   HANDLE hProcess;
   //SymbolManager symbolManager;
   //ModuleManager moduleManager;
   //MemoryTracker memoryTracker;
   //OptionsManager optionsManager;
}

inline string GetStringFromW(wstring w)
{
   return string(w.begin(), w.end());
}

inline string GetString(_TCHAR* t)
{
   return GetStringFromW(wstring(t));
}

int _tmain(int argc, _TCHAR* argv[])
{
   // analyzer.exe (process id) (log file name)

   //if (argc < 3)
   //   return 1;

   global::optionsManager.LoadOptions();

   string logFile;

   if (argc >= 3)
   {
      global::processId = boost::lexical_cast<int>(GetString(argv[1]));
      logFile = GetString(argv[2]);
   }
   else
   {
      cout << "Enter process ID: ";
      cin >> global::processId;
      logFile = LOG_DATA_FILE_NAME;
   }

   global::hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, global::processId);

   global::symbolManager.Initialize();

   LogProcessor lp;
   lp.Run(logFile);
   global::memoryTracker.DoReports();

   global::symbolManager.Cleanup();

   CloseHandle(global::hProcess);

   return 0;
}

