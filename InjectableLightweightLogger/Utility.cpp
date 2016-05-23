#include "stdafx.h"
#include "Utility.h"

#include <boost/filesystem.hpp>
using boost::filesystem::path;
using std::string;
using std::stringstream;

Utility::Utility(void)
{
}


Utility::~Utility(void)
{
}

std::string Utility::GetFileInThisModulePath(std::string fileName)
{
	char fileNameBuf[1000];
	GetModuleFileNameA(global::thisModule, fileNameBuf, 1000);
	path p(fileNameBuf);
	return p.parent_path().append(fileName).string();
}