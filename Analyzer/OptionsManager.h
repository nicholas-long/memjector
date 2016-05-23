#pragma once

#include "stdafx.h"

//For pairing numbers with strings, i am putting the numbers first for optimization purposes. Namely performance of the < operator within ordered sets.

struct Options
{
	
	struct Settings
	{
		std::vector<std::string> debugSymbolPaths;
		bool outputDebugFile;	//output debugging information to trace through the code of this project
		int maximumCallstackLength;	//only step backward this far into callstacks
	} settings;

	struct Filters
	{
		struct ExcludeFromCallstacks
		{
			std::set<std::string> systemModules;							//list of system modules
			std::set<std::string> ignoredModules;							//don't include these modules in callstacks, but don't exclude the callstacks
			std::set<std::string> ignoredCodeFiles;							
			std::set<std::pair<DWORD, std::string>> ignoredCodeLines;		
		} excludeFromCallstacks;
		struct IgnoreCallstacks
		{
			std::set<std::string> ignoredCodeFiles;							//ignore any callstack which refers to these code files
			std::set<std::pair<DWORD, std::string>> ignoredCodeLines;		//ignore callstacks containing these lines of code
			std::set<std::pair<UINT, std::string>> ignoredModuleOffsets;	//ignore callstacks containing these module offsets
		} ignoreCallstacks;
	} filters;
	
	struct MemoryLeakSettings
	{
		bool trackAllLeaks;		//if false, only do grouping
		bool aggregateTotals;	//show total memory usage and count of leaks instead of specific instances
		bool groupingLeaks;		//group leaks by parent memory blocks containing pointers to other blocks base allocation address
		bool aggregateGrouping;	//don't show a lot of repeated instances of the same callstack within a group, sum them up and report instances
		bool intensiveGrouping;	//TODO: check not just pointers within a group, but if any pointers point to within other allocations for grouping
      int aggregateCallstackGroupLevel; // the level of grouping to do when printing all callstacks
   } memoryLeakSettings;
};


class OptionsManager
{
public:
	OptionsManager(void);
	~OptionsManager(void);
	void LoadOptions(void);
	Options options;
};

