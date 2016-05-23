#include "stdafx.h"
#include "FileLogManager.h"
#include <boost/iostreams/device/file.hpp>

namespace global
{
   boost::mutex FileLogMutex;
}

_FileLogManager::_FileLogManager(void)
{
   isOpen = false;
}


_FileLogManager::~_FileLogManager(void)
{
}

void _FileLogManager::Log(LogRecord& rec)
{
   ThreadProperties& threadInfo = global::ThreadManager.GetThreadProperties();

   if (!threadInfo.CurrentlyLogging)
   {
      threadInfo.CurrentlyLogging = true;
      boost::mutex::scoped_lock lock(global::FileLogMutex);

      if (IsOpen())
      {
         //out.write((char*)&rec, sizeof(LogRecord));
         compressed->write((char*)&rec, sizeof(LogRecord));
      }

      threadInfo.CurrentlyLogging = false;
   }
}

void _FileLogManager::Open(std::string fileName)
{
   boost::mutex::scoped_lock lock(global::FileLogMutex);

   if (!isOpen)
   {
      compressed = new boost::iostreams::filtering_ostream();
      compressed->push(boost::iostreams::zlib_compressor());
      compressed->push(boost::iostreams::file_sink(fileName, std::ios::out | std::ios::binary));
      isOpen = true;
   }

}

void _FileLogManager::Close()
{
   boost::mutex::scoped_lock lock(global::FileLogMutex);
   ThreadProperties& threadInfo = global::ThreadManager.GetThreadProperties();

   bool originalState = threadInfo.CurrentlyLogging;
   threadInfo.CurrentlyLogging = true;
   if (IsOpen())
   {
      //out.flush();
      //out.close();
      isOpen = false;
      delete compressed;
      compressed = NULL;
   }

   threadInfo.CurrentlyLogging = originalState;
}

bool _FileLogManager::IsOpen()
{
   return isOpen;
   //return out.is_open();
}