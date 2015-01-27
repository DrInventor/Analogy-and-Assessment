#pragma once

#include <sqlite3.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#if defined(_WIN32)
#include <direct.h>
#else
#include <string.h>
#include <sys/stat.h>
#endif

class DrInventorSqlitedb{
public:
	long InsertFile(const char *filename, const char *tknfilename);
	DrInventorSqlitedb(const char *dir, const char *dbfile, const char which);
	bool isopen(void);
	std::string GiveFileHome(void);
	void UpdateNodeCount(long graphid, unsigned int relcount, unsigned int conceptcount);
	void DumpDatabase(const char *);
	void UpdateProperties(long graphid, int nounique, const char *highestdelist, int highestdeg, int largconn, int nocomponents, const char *mostcommonverbst);
	~DrInventorSqlitedb();
private:
	std::string filehome;
	bool dbopen;
	char fb; //Which slash (linux/windows)
	bool copyfile(const char *from, const char *to);
	std::string dbfilename;
	bool do_sqlite_exec(const char *query);
	bool do_sqlite_exec(const char *query, char **reterror);
	bool do_sqlite_table(const char *query, std::vector<std::string> *thevals, unsigned int *nrows, unsigned int *ncols, char **reterror);
	bool do_sqlite_table(const char *query, std::vector<std::string> *thevals, unsigned int *nrows, unsigned int *ncols);
	void sqlcreate(void);
	sqlite3 *db;
};
