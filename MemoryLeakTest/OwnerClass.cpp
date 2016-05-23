#include "stdafx.h"
#include "OwnerClass.h"

using namespace std;

OwnerClass::OwnerClass(void)
{
}


OwnerClass::~OwnerClass(void)
{
}

void OwnerClass::generateSomeData()
{
   for (int n = 0; n < 1000; n++)
   {
      stringstream s;
      s << "Data #" << n;
      auto c = new ContainedClass(s.str());
      c->owner = this;
      contents.push_back(c);
   }
}