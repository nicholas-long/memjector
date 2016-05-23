// MemoryLeakTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


#include <iostream>
#include <string>
#include <Windows.h>
using namespace std;

void badAllocation(int n)
{
   wchar_t* temp = new wchar_t[100];
   swprintf_s(temp, 100, _T("Test %d"), n);
   wcout << wstring(temp) << endl;
}
void goodAllocation(int n)
{
   wchar_t* temp = new wchar_t[100];
   swprintf_s(temp, 100, _T("Test %d"), n);
   wcout << wstring(temp) << endl;
   delete[] temp;
}
void testSystemString()
{
   BSTR temp = SysAllocString(_T("testtesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttesttest"));
}

void leakRec(int level)
{
   if (level > 7) return;
   if (level > 3)
   {
      for (int n = 1; n <= 100; n++)
      {
         if (n % 2 == 0) goodAllocation(n);
         else 
         {
            badAllocation(n);
         }
      }
   }
   leakRec(level + 1);
}

int runNestedTests()
{
   int sum = 0;
   for (int n = 0; n < 3; n++)
   {
      OwnerClass * temp = new OwnerClass();
      temp->generateSomeData();
      sum += (int)temp->contents.size();
   }
   return sum;
}

void runSharedPtrTest()
{
   typedef boost::shared_ptr<SmartPtrCycleTest> ptr;
   ptr a = ptr(new SmartPtrCycleTest("a"));
   ptr b = ptr(new SmartPtrCycleTest("b"));
   ptr c = ptr(new SmartPtrCycleTest("c")); // this line should not be reported as a leak.  item C will be freed.
   a->SetPointer(b);
   b->SetPointer(a);
   c->SetPointer(a); // 0 references to C mean it should be freed
   // after returning, this cycle will be leaked
}

int _tmain(int argc, _TCHAR* argv[])
{

   cout << "Current process: " << GetCurrentProcessId() << endl;

   cin.get();

   string end = "yes";
   do
   {
      leakRec(0);

      int doSysAlloc = 0;

      runNestedTests();

      runSharedPtrTest();

      cout << "Current process: " << GetCurrentProcessId() << endl;

      cout << "We can run the leaky code again and leak some more.  Continue? (Y/N): ";
      cin >> end;
   }
   while (end[0] == 'y' || end[0] == 'Y');

   return 0;
}

