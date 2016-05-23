#include "stdafx.h"
#include "SmartPtrCycleTest.h"


SmartPtrCycleTest::SmartPtrCycleTest(std::string _name) : name(_name)
{
}


SmartPtrCycleTest::~SmartPtrCycleTest(void)
{
}

void SmartPtrCycleTest::SetPointer(boost::shared_ptr<SmartPtrCycleTest> other)
{
   ptr = other;
}
