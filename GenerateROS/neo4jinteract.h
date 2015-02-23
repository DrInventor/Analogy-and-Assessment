#pragma once

#include <string>
#include <sstream> 
#include <fstream>
#include <vector>

#include "curl/curl.h"
#include "curl/types.h"
#include "curl/easy.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "commonstructs.h"

#if !(defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__))
#define stricmp strcasecmp
#define printf_s printf
#define fprintf_s fprintf
#define sprintf_s sprintf
#define sscanf_s sscanf
#endif

class Neo4jInteract{
public:
	bool isopen(void);
	bool AddNodetoNeo4j(const char *word, int type, const char *gvlabel, long *nodeid, long graphid, long sentid);
	bool MakeLink(int type, long nodeid, long nodeto);
	Neo4jInteract(const char *location);
	bool neo4cypher(const char *command);
	bool GVtoNeo4j(const char *filename, long graphid);
private:
	bool curlok;
	struct MemoryStruct {
		char *memory;
		size_t size;
		MemoryStruct(void){
			memory = (char *)malloc(1);
			size = 0;
		}
	};
	static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
	{
		size_t realsize = size * nmemb;
		struct MemoryStruct *mem = (struct MemoryStruct *)userp;

		mem->memory = (char *)realloc(mem->memory, mem->size + realsize + 1);
		if (mem->memory == NULL) {
			/* out of memory! */
			printf("not enough memory (realloc returned NULL)\n");
			return 0;
		}

		memcpy(&(mem->memory[mem->size]), contents, realsize);
		mem->size += realsize;
		mem->memory[mem->size] = 0;

		return realsize;
	}


	std::string neo4jlocation;
};