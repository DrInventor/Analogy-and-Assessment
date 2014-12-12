#pragma once

#include <string>

struct anode{
	long nodeid;
	std::string word;
};

struct link{
	unsigned int vb, sbj, obj;
};
