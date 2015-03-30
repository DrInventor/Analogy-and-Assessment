#pragma once

#include <string>
#include <vector>

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
	long sentid;
	link(void){
		sentid = 0;
	}	
};

struct triplesent{
	std::string vb, sbj, obj;
	long sentid;
};

struct wordwithid{
	std::string word;
	long id;
};

struct agraph{
	std::vector<struct wordwithid> concepts, relations;
	std::vector<struct link> links;
};

void replaceinstring(std::string &toedit, const char *rpl, const char *with);
std::vector<std::string> splitbydelimiter(std::string tosplit, std::string delim, bool usequotes);