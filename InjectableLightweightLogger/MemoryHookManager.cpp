#include "stdafx.h"
#include "MemoryHookManager.h"

using std::set;

#ifdef WIN32

//Utility functions for hacks
void ApiHook(BYTE* src, BYTE* dest, BYTE** originalAddress){
	DWORD dwback;

	DWORD jmpDiff = (dest - src);
	BYTE* longJump = src - 5;
	DWORD* jmpDiffAddy = (DWORD*)(longJump + 1);
	BYTE* shortJmp = src;

	VirtualProtect(src - 5, 7, PAGE_EXECUTE_READWRITE, &dwback);

	//set long jump before func
	*longJump = 0xe9;
	*jmpDiffAddy = jmpDiff;

	//set short jump in func inital nop
	*((unsigned short*)shortJmp) = 0xF9EB; // jmp -7

	//VirtualProtect(src - 5, 7, dwback, &dwback); // see if this breaks

	*originalAddress = (src + 2);
}

void ResetApiHook(BYTE* func)
{
	DWORD dwback;
	VirtualProtect(func, 2, PAGE_EXECUTE_READWRITE, &dwback);
	*((unsigned short*)func) = 0xFF8B;
	VirtualProtect(func, 2, dwback, &dwback);
}

#endif //WIN32

////////////////////////////////////


MemoryHooking::MemoryHooking(void)
{
}


MemoryHooking::~MemoryHooking(void)
{
}


void MemoryHooking::InsertHook(void* func, void* hookToFunction, void** originalAddress)
{
	HookType h(func, hookToFunction);
	hooksSet.insert(h);
	BYTE* address = (BYTE*)func;
	ApiHook(address, (BYTE*)h.hookToFunction, (BYTE**)originalAddress);
}


bool MemoryHooking::RemoveHooks(void)
{
	set<MemoryHooking::HookType> removed;
	std::for_each(hooksSet.begin(), hooksSet.end(), [&](const HookType& x){
		ResetApiHook((BYTE*)x.func);
	});
	return true;
}
