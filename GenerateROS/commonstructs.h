#pragma once

#include <string>

struct anode{
	long nodeid;
	std::string word;
	long sentid;
	long extra;
	anode(void){
		sentid = 0;
		nodeid = 0;
		extra = 0;
	}
};

struct link{
	unsigned int vb, sbj, obj;
};

struct triplesent{
	std::string vb, sbj, obj;
	long sentid;
};