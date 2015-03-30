#pragma once
#include <stdio.h>
#include <tinydir.h>

#include "neo4jinteract.h"
#include "graphinterface.h"
#include "commonstructs.h"
#include "drinventorsql.h"
#include "rosfromtriples.h"
#include "drinventorfindtriples.h"
#include "graphproperties.h"
//#include "javawrappers.h"

//Make sure correct folder slash is used in Linux or Windows (and some other function changes)
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
static const char slash = '\\';
#else
static const char slash = '/';
#define stricmp strcasecmp
#define printf_s printf
#define fprintf_s fprintf
#define sprintf_s sprintf
#define sscanf_s sscanf
#endif

//bool libcurlSaveFile(const wchar_t *url, const wchar_t *filename);
//std::string computeMD5(const char *filename);
//int DatabasetoCSV(const char *);

struct settings{
	std::string neo4j, store, dbfil, gravz, neouser, neopass;

	settings(void){
		neo4j = neouser = neopass = store = dbfil = gravz = "";
	};
};

bool parsesettings(struct settings *aset);
int ProcessGraph(const char *, DrInventorSqlitedb *sqlitedb, const char *gvloc, Neo4jInteract *neo4db, bool  dogv, std::string tokenfile="", bool oldtkn=false, bool tc=false);
long ProcessGraphWrap(const char *argument, const char *id);
int ProcessAllGraphs(DrInventorSqlitedb *sqlitedb, const char *graphviz, const char *neo4jlo, bool dogv, const char *neopass, const char *neouser);
int KeepSentences(int argc, char** argv, bool range=true);

void TestingStuff(struct settings *set);
//void GraphTesting(char **argv);