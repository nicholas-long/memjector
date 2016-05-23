#include "stdafx.h"
#include "Utility.h"

#include <boost/filesystem.hpp>
using boost::filesystem::path;
using std::string;
using std::stringstream;

#include <TlHelp32.h>

Utility::Utility(void)
{
}


Utility::~Utility(void)
{
}


std::string Utility::GetFileInThisModulePath(std::string fileName)
{
	char fileNameBuf[1000];
	GetModuleFileNameA(0, fileNameBuf, 1000);
	path p(fileNameBuf);
	return p.parent_path().append(fileName).string();
}

std::string Utility::JoinStrings(std::vector<std::string> strs, std::string separator)
{
	stringstream ss;
	bool first = true;
	for (auto s: strs)
	{
		if (!first) ss << separator;
		ss << s;
		first = false;
	}
	return ss.str();
}

std::vector<DWORD> Utility::SuspendOtherThreads()
{
	std::vector<DWORD> results;
	HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (h != INVALID_HANDLE_VALUE)
	{
		THREADENTRY32 te;
		te.dwSize = sizeof(te);
		if (Thread32First(h, &te))
		{
			do
			{
				if (te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(te.th32OwnerProcessID))
				{
					if (te.th32ThreadID != GetCurrentThreadId() && te.th32OwnerProcessID == GetCurrentProcessId())
					{
						HANDLE thread = ::OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
						if (thread != NULL)
						{
							SuspendThread(thread);
							CloseHandle(thread);
							results.push_back(te.th32ThreadID);
						}
					}
				}
				te.dwSize = sizeof(te);
			} while (Thread32Next(h, &te));
		}
	}
	return results;
}

void Utility::EnableOtherThreads(std::vector<DWORD> suspendedThreads)
{
	for (DWORD threadId: suspendedThreads)
	{
		HANDLE thread = ::OpenThread(THREAD_ALL_ACCESS, FALSE, threadId);
		if (thread != NULL)
		{
			ResumeThread(thread);
			CloseHandle(thread);
		}
	}
}