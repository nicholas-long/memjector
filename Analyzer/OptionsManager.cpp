#include "stdafx.h"
#include "OptionsManager.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/lexical_cast.hpp>
//#include <boost/filesystem.hpp>

using std::string;
using std::vector;
using std::set;
using std::pair;
using std::ifstream;
using boost::property_tree::ptree;
using boost::property_tree::json_parser::read_json;


namespace global
{
	OptionsManager optionsManager;
}


OptionsManager::OptionsManager(void)
{
}


OptionsManager::~OptionsManager(void)
{
}

void parseStringAndNumberArray(ptree& t, UINT& outLine, string& outString)
{
	const string numeric("0123456789");
	for (auto p: t)
	{
		string temp = p.second.get_value("error");
		if (temp.find_first_not_of(numeric) == temp.npos)
		{
			outLine = boost::lexical_cast<UINT>(temp);
		}
		else
		{
			outString = temp;
		}
	}
}

void OptionsManager::LoadOptions(void)
{
	string path = Utility::GetFileInThisModulePath("config.json");
	ptree root;
	read_json(ifstream(path), root);
	{
		ptree settings = root.get_child("settings");
		auto& s = options.settings;
		s.outputDebugFile = settings.get("outputDebugFile", false);
		s.maximumCallstackLength = settings.get("maximumCallstackLength", 0);
		for (auto it: settings.get_child("debugSymbolPaths")){
			string str = it.second.get_value("");
			s.debugSymbolPaths.push_back(str); //string
		}
	}

	{
		ptree f = root.get_child("filters");
		auto& s = options.filters;
		{
			ptree e = f.get_child("excludeFromCallstacks");
			auto& s = options.filters.excludeFromCallstacks;
			for (auto p: e.get_child("systemModules"))
			{
				s.systemModules.insert(p.second.get_value(""));
			}
			for (auto p: e.get_child("ignoredModules"))
			{
				s.ignoredModules.insert(p.second.get_value(""));
			}
			for (auto p: e.get_child("ignoredCodeFiles"))
			{
				s.ignoredCodeFiles.insert(p.second.get_value(""));
			}
			for (auto p: e.get_child("ignoredCodeLines"))
			{
				string temp;
				UINT num;
				parseStringAndNumberArray(p.second, num, temp);
				s.ignoredCodeLines.insert(std::make_pair((DWORD)num, temp));
			}
		}

		{
			ptree i = f.get_child("ignoreCallstacks");
			auto& s = options.filters.ignoreCallstacks;
			for (auto p: i.get_child("ignoredCodeFiles"))
			{
				s.ignoredCodeFiles.insert(p.second.get_value(""));
			}
			for (auto p: i.get_child("ignoredCodeLines"))
			{
				string temp;
				UINT num;
				parseStringAndNumberArray(p.second, num, temp);
				s.ignoredCodeLines.insert(std::make_pair((DWORD)num, temp));
			}
			for (auto p: i.get_child("ignoredModuleOffsets"))
			{
				string temp;
				UINT num;
				parseStringAndNumberArray(p.second, num, temp);
				s.ignoredModuleOffsets.insert(std::make_pair(num, temp));
			}

		}
	}

	{
		ptree m = root.get_child("memoryLeakSettings");
		auto& s = options.memoryLeakSettings;
		s.trackAllLeaks = m.get("trackAllLeaks", true);
		s.aggregateTotals = m.get("aggregateTotals", true);
		s.groupingLeaks = m.get("groupingLeaks", false);
		s.aggregateGrouping = m.get("aggregateGrouping", false);
		s.intensiveGrouping = m.get("intensiveGrouping", false);
      s.aggregateCallstackGroupLevel = m.get("aggregateCallstackGroupLevel", 0);
	}
}
