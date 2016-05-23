#pragma once

#include <memory>
#include <utility>
#include <ostream>
#include <vector>

struct TrackedCallstack
{
   typedef std::shared_ptr<TrackedCallstack> ptr;
   typedef std::pair<DWORD, std::string> FileLinePair;
   std::vector<UINT> callstackAddresses;
   std::vector<UINT> shortenedAddresses;
   std::set<std::string> codeFilesMentionedInCallstack;
   std::set<FileLinePair> codeFileLinePairs;
   std::vector<std::string> callstackVerbose;					//the displayed callstack
   std::set<std::string> moduleFilesMentionedInCallstack;
   std::set<FileLinePair> moduleRelativeAddressesInCallstack;
   bool ignoredByFilters;
};

typedef std::map<std::vector<UINT>, TrackedCallstack::ptr> UniqueCallstackCacheType;

class MemoryTracker
{
public:
   MemoryTracker(void);
   ~MemoryTracker(void);
   void trackAllocation(void* mem, SIZE_T bytes, std::vector<UINT> callstackFunctions);
   void trackFree(void* mem); // std::vector<void*> callstackFunctions

   void printDebugAllocations();

   struct TrackedAllocation
   {
      bool operator<(const TrackedAllocation& other) const
      {
         return mem < other.mem;
      };
      void* mem;
      SIZE_T bytes;
      //std::vector<std::string> callstack; //old
      TrackedCallstack::ptr callStack;
      unsigned long long uid;
   };

   void DoReports();

private:
   bool ApplyIgnoreFilters(TrackedCallstack::ptr t);

   void ReportAllLeaks(std::ostream& out, bool aggregate);
   void ReportGroupLeaks(std::ostream& out, bool intensive, bool aggregate);

   void FilterOutFalsePositives();

   static bool InvalidAllocation(const TrackedAllocation& ta);

   bool doneReports;
   std::set<TrackedAllocation> trackedAllocations;
   UniqueCallstackCacheType cache;

   unsigned long long uid;
};

