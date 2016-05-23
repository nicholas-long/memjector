#pragma once

class SmartPtrCycleTest
{
public:
   SmartPtrCycleTest(std::string);
   ~SmartPtrCycleTest(void);

   void SetPointer(boost::shared_ptr<SmartPtrCycleTest> other);

   boost::shared_ptr<SmartPtrCycleTest> ptr;
   std::string name;
};

