#pragma once
class Utility
{
public:
	Utility(void);
	~Utility(void);
	static std::string GetFileInThisModulePath(std::string fileName);
	static std::string JoinStrings(std::vector<std::string> strs, std::string separator);
	static std::vector<DWORD> SuspendOtherThreads();
	static void EnableOtherThreads(std::vector<DWORD> suspendedThreads);
};

