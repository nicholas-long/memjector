#pragma once

#include "../Common/LogRecord.h"
#include <fstream>
#include <string>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>

class _FileLogManager
{

public:
   _FileLogManager(void);
   ~_FileLogManager(void);

   void Log(LogRecord& rec);
   void Open(std::string fileName);
   void Close();

private:
   bool IsOpen();
   //std::ofstream out;
   boost::iostreams::filtering_ostream* compressed;
   bool isOpen;
};

