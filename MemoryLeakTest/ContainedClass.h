#pragma once

#include "OwnerClass.h"

#include <string>

class ContainedClass
{
public:
   ContainedClass(std::string data_);
   ~ContainedClass(void);
   std::string data;
   OwnerClass* owner;
};

