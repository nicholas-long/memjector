#pragma once

#include <set>
#include <string>

class MemoryHooking
{
public:
	MemoryHooking(void);
	~MemoryHooking(void);
	bool RemoveHooks(void);
	void InsertHook(void* func, void* hookToFunction, void** originalAddress);

	struct HookType
	{
		HookType(void* func_, void* hookToFunction_): func(func_), hookToFunction(hookToFunction_)
		{};
		bool operator<(const HookType& other) const
		{
			return func < other.func;
		};
		void* func;
		void* hookToFunction;
	};

	std::set<HookType> hooksToAdd, hooksSet;
};
