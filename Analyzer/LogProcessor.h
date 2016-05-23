#pragma once
#include "../Common/LogRecord.h"

class LogProcessor
{
public:
   LogProcessor(void);
   ~LogProcessor(void);
private:
   void Process(LogRecord rec);
   std::string fileName;
public:
   void Run(std::string fileName);
};

