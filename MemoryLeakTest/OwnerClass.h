#pragma once

#include <vector>
class ContainedClass;

class OwnerClass
{
public:
	OwnerClass(void);
	~OwnerClass(void);
	std::vector<ContainedClass*> contents;

	void generateSomeData();
};

