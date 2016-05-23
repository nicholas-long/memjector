#include "stdafx.h"
#include "MemoryTracker.h"
#include <boost/filesystem.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <fstream>

using std::vector;
using std::string;
using std::stringstream;
using std::ofstream;
using std::set;
using std::map;
using std::hex; using std::dec; using std::endl;

namespace global
{
   MemoryTracker memoryTracker;
}

MemoryTracker::MemoryTracker(void): doneReports(false)
{
}

MemoryTracker::~MemoryTracker(void)
{
}



UniqueCallstackCacheType callstackCache;

typedef boost::recursive_mutex::scoped_lock ScopedRecursiveLock;
boost::recursive_mutex allocationTrackingMutex;

//string getAddressDescription(UINT codeAddress)
//{
//	string fileName;
//	UINT lineNum;
//	if (global::symbolManager.GetFileNameAndLineForAddress(codeAddress, fileName, lineNum))
//	{
//		std::stringstream s;
//		s << fileName << "(" << lineNum << ")";
//		return s.str();
//	}
//	else
//	{
//		return global::moduleManager.GetModuleNameFromAddress(codeAddress);
//	}
//}

string fileNameFromPath(string fullPath)
{
   boost::filesystem::path p(fullPath);
   return p.filename().string();
}

//TrackedCallstack::ptr MemoryTracker::examinedCallstack(std::vector<UINT> callstackFunctions)
//{
//	TrackedCallstack::ptr t = TrackedCallstack::ptr(new TrackedCallstack());
//
//	bool filterExcludeThis = false;
//	for (UINT address: callstackFunctions)
//	{
//		bool filteredLine = false;
//
//
//
//	}
//
//	return t;
//}

bool MemoryTracker::ApplyIgnoreFilters(TrackedCallstack::ptr t)
{
   auto& ignored = global::optionsManager.options.filters.ignoreCallstacks;
   for (auto f: t->codeFilesMentionedInCallstack)
   {
      if (ignored.ignoredCodeFiles.find(f) != ignored.ignoredCodeFiles.end()) return true;
   }
   for (auto p: t->codeFileLinePairs)
   {
      if (ignored.ignoredCodeLines.find(p) != ignored.ignoredCodeLines.end()) return true;
   }
   for (auto p: t->moduleRelativeAddressesInCallstack)
   {
      if (ignored.ignoredModuleOffsets.find(p) != ignored.ignoredModuleOffsets.end()) return true;
   }
   return false;
}

void MemoryTracker::trackAllocation(void* mem, SIZE_T bytes, std::vector<UINT> callstackFunctions)
{
   ScopedRecursiveLock(allocationTrackingMutex); //lock
   vector<UINT> callStackNext(++callstackFunctions.begin(), callstackFunctions.end()); //subtract call to HeapAlloc

   TrackedAllocation a;
   a.mem = mem;
   a.bytes = bytes;
   a.uid = this->uid++;

   auto& options = global::optionsManager.options;

   UniqueCallstackCacheType::iterator it = callstackCache.find(callStackNext);

   if (it == callstackCache.end()) //look up the callstack based on memory addresses alone to see if we already cached it
   {
      TrackedCallstack::ptr t = TrackedCallstack::ptr(new TrackedCallstack());

      //std::set<std::string> codeFilesMentionedInCallstack;
      //std::set<FileLinePair> codeFileLinePairs;
      //std::vector<std::string> callstackVerbose;

      auto& excluded = options.filters.excludeFromCallstacks;

      //std::transform(callStackNext.begin(), callStackNext.end(), std::back_inserter(t->callstackVerbose), [&](UINT codeAddress){
      for (UINT codeAddress: callStackNext)
      {
         bool filteredThis = options.settings.maximumCallstackLength > 0 && options.settings.maximumCallstackLength <= (int)t->callstackVerbose.size();
         string fileName;
         UINT lineNum;
         if (global::symbolManager.GetFileNameAndLineForAddress(codeAddress, fileName, lineNum))
         {
            string fileShortName = fileNameFromPath(fileName);
            TrackedCallstack::FileLinePair codeFileLinePair((DWORD)lineNum, fileShortName);
            t->codeFileLinePairs.insert(codeFileLinePair);
            t->codeFilesMentionedInCallstack.insert(fileShortName);

            if (excluded.ignoredCodeFiles.find(fileShortName) != excluded.ignoredCodeFiles.end()) filteredThis = true;
            if (excluded.ignoredCodeLines.find(codeFileLinePair) != excluded.ignoredCodeLines.end()) filteredThis = true;

            if (!filteredThis)
            {
               std::stringstream s;
               s << fileShortName << "(" << lineNum << ")";
               t->callstackVerbose.push_back(s.str());
            }
         }
         else
         {
            try
            {
               auto& modinfo = global::moduleManager.GetModuleInfoFromAddress(codeAddress);
               TrackedCallstack::FileLinePair moduleOffsetPair((DWORD)(codeAddress - modinfo.base), modinfo.fileName);
               t->moduleRelativeAddressesInCallstack.insert(moduleOffsetPair);
               if (options.filters.excludeFromCallstacks.ignoredModules.find(modinfo.fileName) != options.filters.excludeFromCallstacks.ignoredModules.end())
                  filteredThis = true;
            }
            catch(...)
            {}
            if (!filteredThis)
            {
               t->callstackVerbose.push_back("? " + global::moduleManager.GetModuleNameFromAddress(codeAddress));
            }
         }
      }
      //});

      t->callstackAddresses = callStackNext;

      //apply filters to determine if we should ignore this callstack
      t->ignoredByFilters = ApplyIgnoreFilters(t);

      callstackCache[callStackNext] = t;
      a.callStack = t;
   }
   else
   {
      a.callStack = it->second;
   }

   trackedAllocations.insert(a);
}


void MemoryTracker::trackFree(void* mem) //, std::vector<void*> callstackFunctions
{
   ScopedRecursiveLock(allocationTrackingMutex); //lock
   TrackedAllocation a;
   a.mem = mem;
   trackedAllocations.erase(a);
}

bool MemoryTracker::InvalidAllocation(const TrackedAllocation& ta)
{
   char* buf = new char[ta.bytes];
   SIZE_T numberOfBytesRead;
   BOOL result = ReadProcessMemory(global::hProcess, (LPCVOID)ta.mem, buf, ta.bytes, &numberOfBytesRead);
   delete[] buf;
   return !result; // nonzero = success
}

void MemoryTracker::FilterOutFalsePositives()
{
   set<const TrackedAllocation> next;
   std::remove_copy_if(trackedAllocations.begin(), trackedAllocations.end(), std::inserter(next, next.begin()), InvalidAllocation);
   trackedAllocations.clear();
   std::copy(next.begin(), next.end(), std::inserter(trackedAllocations, trackedAllocations.begin()));
}

void MemoryTracker::printDebugAllocations()
{
   //typedef std::map<TrackedCallstack::ptr, vector<TrackedAllocation>> ResultMap;
   //ResultMap resultMap;
   ////auto& dbg = global::debugStream;
   //std::for_each(trackedAllocations.begin(), trackedAllocations.end(), [&](TrackedAllocation a){
   //	TrackedAllocation copy = a;
   //	copy.callStack = TrackedCallstack::ptr();
   //	resultMap[a.callStack].push_back(copy);
   //});

   //dbg << resultMap.size() << " unique allocation callstacks" << std::endl;

   //std::for_each(resultMap.begin(), resultMap.end(), [&](ResultMap::value_type m){
   //	TrackedCallstack::ptr c = m.first;
   //	if (!c->ignoredByFilters)
   //	{
   //		vector<TrackedAllocation>& allocs = m.second;
   //		UINT sum = 0;
   //		std::for_each(allocs.begin(), allocs.end(), [&](TrackedAllocation a){
   //			sum += a.bytes;
   //		});

   //		dbg << sum << " bytes, callstack: " << std::endl;
   //		std::for_each(c->callstackVerbose.begin(), c->callstackVerbose.end(), [&](std::string s){
   //			dbg << "\t" << s << std::endl;
   //		});
   //	}
   //});

   ////std::for_each(trackedAllocations.begin(), trackedAllocations.end(), [&](const TrackedAllocation& a){
   ////	dbg << std::hex << ((DWORD)a.mem) << ", " << std::dec << a.bytes << " bytes" << std::endl;
   ////	std::for_each(a.callStack->callstackVerbose.begin(), a.callStack->callstackVerbose.end(), [&](std::string s){
   ////		dbg << "\t" << s << std::endl;
   ////	});
   ////});
   //dbg.close();
}

void MemoryTracker::DoReports()
{
   ScopedRecursiveLock(allocationTrackingMutex); //lock
   //if (doneReports) return;

   auto& settings = global::optionsManager.options.memoryLeakSettings;

   FilterOutFalsePositives();

   if (settings.trackAllLeaks)
   {
      ofstream out(Utility::GetFileInThisModulePath("MemoryLeaks.txt"));
      ReportAllLeaks(out, settings.aggregateTotals);
   }
   if (settings.groupingLeaks)
   {
      ofstream out(Utility::GetFileInThisModulePath("GroupedLeaks.txt"));
      ReportGroupLeaks(out, settings.intensiveGrouping, settings.aggregateGrouping);
   }

   //doneReports = true;
}

vector<string> getCallstackLevel(int count, MemoryTracker::TrackedAllocation& a)
{
   vector<string> result;
   auto& csv = a.callStack->callstackVerbose;

   copy_if(csv.begin(), csv.end(), std::back_inserter(result), [&](string s){ 
      return s.size() > 0 && s[0] != '?' && result.size() < (size_t)count;
   });

   if (result.size()) return result;

   copy_if(csv.begin(), csv.end(), std::back_inserter(result), [&](string s){
      return result.size() < (size_t)count;
   });

   return result;
}

void MemoryTracker::ReportAllLeaks(std::ostream& out, bool aggregate)
{
   if (aggregate)
   {
      typedef std::map<TrackedCallstack::ptr, vector<TrackedAllocation>> ResultMap;
      ResultMap resultMap;

      int aggregateLevel = global::optionsManager.options.memoryLeakSettings.aggregateCallstackGroupLevel;

      map<vector<string>, vector<TrackedAllocation>> trackedAllocationsByLevel;

      for (TrackedAllocation a: trackedAllocations)
      {
         TrackedAllocation copy = a;
         copy.callStack = TrackedCallstack::ptr();
         if (aggregateLevel <= 0)
         {
            resultMap[a.callStack].push_back(copy);
         }
         else
         {
            vector<string> smallCallstack = getCallstackLevel(aggregateLevel, a);
            trackedAllocationsByLevel[smallCallstack].push_back(a);
         }
      }

      if (aggregateLevel <= 0)
      {
         out << resultMap.size() << " unique allocation callstacks" << std::endl;

         for (ResultMap::value_type m: resultMap)
         {
            TrackedCallstack::ptr c = m.first;
            if (!c->ignoredByFilters)
            {
               vector<TrackedAllocation>& allocs = m.second;
               UINT sum = 0;

               for (TrackedAllocation a: allocs)
               {
                  sum += a.bytes;
               }

               out << sum << " bytes, callstack: " << std::endl;
               for (std::string s: c->callstackVerbose)
               {
                  out << "\t" << s << std::endl;
               }
            }
         }
      }
      else // aggregateLevel > 0
      {
         vector<vector<string>> outValues;
         map<vector<string>, UINT> totalBytes;

         for (auto& x : trackedAllocationsByLevel)
         {
            outValues.push_back(x.first);
            UINT sum = 0;
            for (TrackedAllocation& a: x.second)
               sum += a.bytes;
            totalBytes[x.first] = sum;
         }
         
         std::sort(outValues.begin(), outValues.end(), [&](vector<string> a, vector<string> b){
            return totalBytes[a] > totalBytes[b];
         });

         for (auto vec : outValues)
         {
            out << totalBytes[vec] << " bytes, callstack: " << std::endl;
            for (std::string s : (*trackedAllocationsByLevel[vec].begin()).callStack->callstackVerbose)
               out << "\t" << s << std::endl;
         }

      }
   }
   else
   {
      for (auto& a: trackedAllocations)
      {
         out << std::hex << ((DWORD)a.mem) << ", " << std::dec << a.bytes << " bytes" << std::endl;
         for (std::string s: a.callStack->callstackVerbose)
         {
            out << "\t" << s << std::endl;
         }
      }
   }
}

struct ReferenceCount
{
   ReferenceCount(): totalSize(0), instances(0)
   {};
   UINT totalSize;
   UINT instances;
};

typedef map<UINT, set<UINT>> ParentRefMap;
typedef ParentRefMap ChildRefMap;
typedef set<UINT> VisitedMap;
typedef set<UINT> AddressSet;
typedef map<UINT, MemoryTracker::TrackedAllocation> MapAddressToAllocation;
typedef map<TrackedCallstack::ptr, ReferenceCount> PointerTracking;
typedef map<TrackedCallstack::ptr, UINT> TrackingSingleAddress;


void checkMemoryGrouping(const MemoryTracker::TrackedAllocation& ta, AddressSet& addresses, ParentRefMap& parentMap, ChildRefMap& childMap)
{
   //UINT * memory = (UINT*)address, *endMemory = (UINT*)(address+size);	//use memory hacks to see if there's pointers to the allocations in here

   UINT * buf = new UINT[ta.bytes];
   ZeroMemory(buf, ta.bytes);

   SIZE_T numberOfBytesRead = 0;

   if (ReadProcessMemory(global::hProcess, ta.mem, buf, ta.bytes, &numberOfBytesRead))
   {
      UINT * memory = buf;
      UINT * endMemory = buf + (ta.bytes / sizeof(UINT)); // min( (ta.bytes / sizeof(UINT)), (numberOfBytesRead / sizeof(UINT)) );
      for (; memory < endMemory; memory++)
      {
         auto it = addresses.find(*memory);
         if (it != addresses.end())
         {
            parentMap[*it].insert((UINT)ta.mem);
            childMap[(UINT)ta.mem].insert(*it);
         }
      }
   }

   delete[] buf;
}

//recursive
void walkUpGrouping(UINT address, ParentRefMap& parentMap, AddressSet& outAddresses, AddressSet& visited)
{
   visited.insert(address);
   if (parentMap[address].size() == 0)	//base case
   {
      outAddresses.insert(address);
   }
   else
   {
      for (UINT a: parentMap[address])
      {
         if (visited.find(a) == visited.end())
         {
            walkUpGrouping(a, parentMap, outAddresses, visited);
         }
      }
   }
}

// remove cycles
void cleanUpGroupingRec(UINT address, ParentRefMap& parentMap, ChildRefMap& childMap, MapAddressToAllocation& addressesToAllocations, AddressSet& visited)
{
   if (visited.find(address) != visited.end()) return;

   visited.insert(address);

   auto myUid = addressesToAllocations[address].uid;
   auto parents = parentMap[address]; // copy
   for (auto parent : parents)
   {
      auto parentUid = addressesToAllocations[parent].uid;
      
      bool eraseParent = 
         (parentUid > myUid) || 
         (address == parent); // remove self references
      
      if (eraseParent)
      {
         parentMap[address].erase(parent);
         childMap[parent].erase(address);
      }
      else
      {
         cleanUpGroupingRec(parent, parentMap, childMap, addressesToAllocations, visited);
      }
   }
}

void cleanUpSingleGrouping(UINT address, ParentRefMap& parentMap, ChildRefMap& childMap, MapAddressToAllocation& addressesToAllocations)
{
   auto myUid = addressesToAllocations[address].uid;
   auto parents = parentMap[address]; // copy
   for (auto parent : parents)
   {
      auto parentUid = addressesToAllocations[parent].uid;
      
      bool eraseParent = 
         (parentUid > myUid) || 
         (address == parent); // remove self references
      
      if (eraseParent)
      {
         parentMap[address].erase(parent);
         childMap[parent].erase(address);
      }
   }
}

void cleanUpAllGroupings(std::set<MemoryTracker::TrackedAllocation>& trackedAllocations, ParentRefMap& parentMap, ChildRefMap& childMap, MapAddressToAllocation& addressesToAllocations)
{
   //std::set<UINT> visited, frontier;

   for (auto& a : trackedAllocations)
   {
      //UINT addy = (UINT)a.mem;
      //visited.insert(addy);

      cleanUpSingleGrouping((UINT)a.mem, parentMap, childMap, addressesToAllocations);
      
      //for (UINT next : parentMap[addy])
      //   if (visited.find(next) == visited.end())
      //      frontier.insert(next);
   }
}

inline std::ostream& indent(std::ostream& out, int count)
{
   for (int n = 0; n < count; n++) out << "  ";
   return out;
}

UINT findResponsibleBytes(UINT address, MapAddressToAllocation& addressToAllocationMap, ChildRefMap& childMap, AddressSet& visitedInThisSearch)
{
   if (visitedInThisSearch.find(address) != visitedInThisSearch.end()) return 0;
   visitedInThisSearch.insert(address);
   UINT sum = 0;

   sum += addressToAllocationMap[address].bytes;

   for (UINT c: childMap[address])
   {
      sum += findResponsibleBytes(c, addressToAllocationMap, childMap, visitedInThisSearch);
   }
   return sum;
}

void printGroupings(std::ostream& out, int level, UINT address, MapAddressToAllocation& addressToAllocationMap, ChildRefMap& childMap, AddressSet& visited)
{
   if (visited.find(address) != visited.end()) return;
   visited.insert(address);
   auto& alloc = addressToAllocationMap[address];
   auto p = alloc.callStack;

   indent(out, level) << "Allocation at " << hex << address << " " << dec << alloc.bytes << " bytes; Callstack:" << endl;
   for (auto s: p->callstackVerbose)
   {
      indent(out, level) << s << endl;
   }

   for (auto a: childMap[address])
   {
      printGroupings(out, level + 1, a, addressToAllocationMap, childMap, visited);
   }
}

//forward declaration
void printAggregateGroupings(std::ostream& out, int level, set<UINT>& addresses, MapAddressToAllocation& addressToAllocationMap, ChildRefMap& childMap, AddressSet visited);

void printSingleAggregateGrouping(std::ostream& out, int level, UINT address, TrackedCallstack::ptr callstackPtr, ReferenceCount& rc, MapAddressToAllocation& addressToAllocationMap, ChildRefMap& childMap, AddressSet visited, UINT sumResponsible)
{
   if (visited.find(address) != visited.end()) return;
   visited.insert(address);
   auto& alloc = addressToAllocationMap[address];
   auto p = callstackPtr;

   indent(out, level) << "Allocations " << dec << rc.totalSize << " total bytes for " << rc.instances << " instances; Responsible for " << sumResponsible << " nested bytes. Callstack:" << endl;
   for (auto s: p->callstackVerbose)
   {
      indent(out, level) << "|" << s << endl;
   }
   printAggregateGroupings(out, level + 1, childMap[address], addressToAllocationMap, childMap, visited);
}

void printAggregateGroupings(std::ostream& out, int level, AddressSet& addresses, MapAddressToAllocation& addressToAllocationMap, ChildRefMap& childMap, AddressSet visited)
{
   PointerTracking pointerMap, responsibleMap;
   TrackingSingleAddress pointerAddressMap;
   for (auto a: addresses)
   {
      auto& alloc = addressToAllocationMap[a];
      auto p = alloc.callStack;
      pointerAddressMap[p] = a;
      pointerMap[p].instances++;
      pointerMap[p].totalSize += alloc.bytes;
      {
         AddressSet visitedTemp;
         responsibleMap[p].totalSize += findResponsibleBytes(a, addressToAllocationMap, childMap, visitedTemp);
      }
   }
   for (auto p: pointerMap)
   {
      printSingleAggregateGrouping(out, level, pointerAddressMap[p.first], p.first, pointerMap[p.first], addressToAllocationMap, childMap, visited, responsibleMap[p.first].totalSize); 
   }
}

void MemoryTracker::ReportGroupLeaks(std::ostream& out, bool intensive, bool aggregate)
{
   AddressSet allAddresses;
   MapAddressToAllocation addressesToAllocations;
   for (auto& a: trackedAllocations)	//get all addresses
   {
      allAddresses.insert((UINT)a.mem);
      addressesToAllocations[(UINT)a.mem] = a;
   }

   //Find the groupings.
   ParentRefMap parentMap;
   ChildRefMap childMap;
   for (auto& a: trackedAllocations)
   {
      checkMemoryGrouping(a, allAddresses, parentMap, childMap);
   }

   //// clean up cycles
   //for (auto& a: trackedAllocations)
   //   cleanUpGroupingRec((UINT)a.mem, parentMap, childMap, addressesToAllocations, AddressSet());
   cleanUpAllGroupings(trackedAllocations, parentMap, childMap, addressesToAllocations);

   //find all the root addresses - recursive
   AddressSet rootAddresses;
   {
      AddressSet visited;
      for (auto& a: trackedAllocations)
      {
         walkUpGrouping((UINT)a.mem, parentMap, rootAddresses, visited);
      }
   }

   AddressSet visited;
   if (aggregate)
   {
      printAggregateGroupings(out, 0, rootAddresses, addressesToAllocations, childMap, visited);
   }
   else
   {
      for (UINT a: rootAddresses)
      {
         printGroupings(out, 0, a, addressesToAllocations, childMap, visited);
      }
   }
}