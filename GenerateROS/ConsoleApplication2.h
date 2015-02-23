#include <stdio.h>
#include <tinydir.h>

#include "neo4jinteract.h"
#include "graphinterface.h"
#include "commonstructs.h"
#include "drinventorsql.h"
#include "rosfromtriples.h"
#include "drinventorfindtriples.h"
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
int ProcessGraph(const char *, DrInventorSqlitedb *sqlitedb, const char *, const char *, bool tc=false);
int ProcessAllGraphs(DrInventorSqlitedb *sqlitedb, const char *graphviz, const char *neo4jlo);
int KeepSentences(int argc, char** argv, bool range=true);