#include <stdio.h>
#include <tchar.h>

#include <tinydir.h>


#include "neo4jinteract.h"
#include "graphinterface.h"
#include "commonstructs.h"
#include "drinventorsql.h"
#include "rosfromtriples.h"
#include "drinventorfindtriples.h"


std::vector<std::string> splitbydelimiter(std::string tosplit, std::string delim, bool usequotes = false);
void replaceinstring(std::string &toedit, const char *rpl, const char *with);
//bool libcurlSaveFile(const wchar_t *url, const wchar_t *filename);
//std::string computeMD5(const char *filename);
//int DatabasetoCSV(const char *);

struct settings{
	std::string neo4j, store, dbfil, gravz;

	settings(void){
		neo4j = store = dbfil = gravz = "";
	};
};

bool parsesettings(struct settings *aset);
int ProcessGraph(const char *, DrInventorSqlitedb *sqlitedb, const char *, const char *);
int ProcessAllGraphs(DrInventorSqlitedb *sqlitedb, const char *graphviz, const char *neo4jlo);