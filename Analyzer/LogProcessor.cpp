#include "stdafx.h"
#include "LogProcessor.h"
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>

using namespace std;

LogProcessor::LogProcessor(void)
{
}


LogProcessor::~LogProcessor(void)
{
}


void LogProcessor::Process(LogRecord rec)
{
   using namespace global;
   switch (rec.RecordType)
   {
   case LogRecord::LOAD_MODULE:
      moduleManager.InformLoadModule(rec.ModuleInfo.ModuleHandle, rec);
      break;
   case LogRecord::UNLOAD_MODULE:
      moduleManager.InformUnloadModule(rec.ModuleInfo.ModuleHandle);
      cout << "Unloaded module " << rec.ModuleInfo.ModuleHandle << endl;
      break;
   case LogRecord::ALLOC:
      {
         std::vector<UINT> callstack;
         for (int n = 0; n < CALLSTACK_SIZE && rec.AllocInfo.Callstack[n] != 0; n++)
            callstack.push_back(rec.AllocInfo.Callstack[n]);

         memoryTracker.trackAllocation(rec.AllocInfo.Memory, rec.AllocInfo.BlockSize, callstack);
      }
      break;
   case LogRecord::FREE:
      memoryTracker.trackFree(rec.AllocInfo.Memory);
      break;
   default:
      cout << "Error processing a record" << endl;
      break;
   }
}


void LogProcessor::Run(std::string fileName)
{
   //ifstream in(fileName, ios::in | ios::binary);
   boost::iostreams::filtering_istream in;
   in.push(boost::iostreams::zlib_decompressor());
   in.push(boost::iostreams::file_source(fileName, std::ios::in | std::ios::binary));

   while (!in.eof())
   {
      LogRecord rec;
      in.read((char*)&rec, sizeof(LogRecord));
      Process(rec);
   }
}
