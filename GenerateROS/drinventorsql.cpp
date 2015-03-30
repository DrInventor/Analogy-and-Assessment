#include "drinventorsql.h"

DrInventorSqlitedb::~DrInventorSqlitedb(){
	sqlite3_close(db);
}

inline bool FileOpenable(const char *filename) {
	FILE *file = fopen(filename, "r");
	if (file) {
		fclose(file);
		return true;
	}
	else {
		return false;
	}
}

DrInventorSqlitedb::DrInventorSqlitedb(const char *dir, const char *dbfile, const char which){
	//Open the sqlite database, if it doesn't exist, create it
	dbfilename = dbfile;
	filehome = dir;
	int rc;
	dbopen = false;
	fb = which;
	if (!FileOpenable(dbfile)){
		rc = sqlite3_open((const char *)dbfile, &db);
		if (rc){
			printf_s("Can't open database: %s\n", sqlite3_errmsg(db));
			sqlite3_close(db);
		}
		else{
			sqlcreate();
			dbopen = true;
		}
	}
	else{
		rc = sqlite3_open((const char *)dbfile, &db);
		if (rc){
			printf_s("Can't open database: %s\n", sqlite3_errmsg(db));
			sqlite3_close(db);
		}
		else
			dbopen = true;
	}
}

DrInventorSqlitedb::DrInventorSqlitedb(const char *dbfile){
	//Open the sqlite database, if it doesn't exist, create it
	dbfilename = dbfile;
	filehome = "";
	int rc;
	dbopen = false;
	fb = '\\';
	if (!FileOpenable(dbfile)){
		rc = sqlite3_open((const char *)dbfile, &db);
		if (rc){
			printf_s("Can't open database: %s\n", sqlite3_errmsg(db));
			sqlite3_close(db);
		}
		else{
			sqlcreate();
			dbopen = true;
		}
	}
	else{
		rc = sqlite3_open((const char *)dbfile, &db);
		if (rc){
			printf_s("Can't open database: %s\n", sqlite3_errmsg(db));
			sqlite3_close(db);
		}
		else
			dbopen = true;
	}
}

void DrInventorSqlitedb::DumpDatabase(const char *filename){
	std::ofstream file(filename);
	if (file.is_open()){
		std::vector<std::string> values;
		unsigned int nrows, ncols;
		if (do_sqlite_table("SELECT * FROM Graphlist", &values, &nrows, &ncols)){
			file << "GID,Filename,TknFilename,No. Concepts,No. Relations,No. Unique Relations,Most Common Verb(s),Node(s) with highest degree,Highest Degree of Node,No.Connected Components,No.Nodes in Largest Connected Component\n";
			for (unsigned int i = 0; i < nrows; ++i){
				for (unsigned int j = 0; j < ncols; ++j){
					file << "\"" << values[j + i*ncols] << "\"";
					if (j != ncols - 1)
						file << ",";
				}
				if (i != nrows - 1)
					file << "\n";
			}
		}
		file.close();
	}
}

bool DrInventorSqlitedb::do_sqlite_table(const char *query, std::vector<std::string> *thevals, unsigned int *nrows, unsigned int *ncols, char **reterror){
	sqlite3_stmt *statement;
	std::vector<std::string>().swap(*thevals);
	*ncols = 0; *nrows = 0;
	if (sqlite3_prepare_v2(db, query, -1, &statement, 0) == SQLITE_OK){
		*ncols = sqlite3_column_count(statement);
		int result = 0;
		/*for (int i = 0; i < *ncols; ++i)
		thevals->emplace_back("Ignore");*/
		while (true){
			result = sqlite3_step(statement);
			if (result == SQLITE_ROW){
				++*nrows;
				for (unsigned int i = 0; i<*ncols; ++i){
					std::string val;
					char *ptr = (char*)sqlite3_column_text(statement, i);
					if (ptr)
						val = ptr;
					else
						val = "";
					thevals->emplace_back(val);
				}
			}
			else
				break;
		}
		sqlite3_finalize(statement);
		return true;
	}
	else{
		std::string error = sqlite3_errmsg(db);
		*reterror = (char *)malloc((error.size() + 1)*sizeof(char));
		strcpy(*reterror, error.c_str());
		return false;
	}
}

bool DrInventorSqlitedb::do_sqlite_table(const char *query, std::vector<std::string> *thevals, unsigned int *nrows, unsigned int *ncols){
	char *error;
	bool returns = do_sqlite_table(query, thevals, nrows, ncols, &error);
	if (!returns)
		free(error);
	return returns;
}

bool DrInventorSqlitedb::do_sqlite_exec(const char *query){
	//Overloaded function if we don't care about the error message
	char *error;
	bool returns = do_sqlite_exec(query, &error);
	if (!returns)
		free(error);
	return returns;
}

bool DrInventorSqlitedb::do_sqlite_exec(const char *query, char **reterror){
	sqlite3_stmt *statement;
	if (sqlite3_prepare_v2(db, (const char *)query, -1, &statement, 0) == SQLITE_OK){
		sqlite3_step(statement);
		sqlite3_finalize(statement);
		return true;
	}
	else{
		std::string error = sqlite3_errmsg(db);
		*reterror = (char *)malloc((error.size() + 1)*sizeof(char));
		strcpy(*reterror, error.c_str());
		sqlite3_finalize(statement);
		return false;
	}
}

void DrInventorSqlitedb::UpdateProperties(long graphid, int nounique, const char *highestdelist, int highestdeg, int largconn, int nocomponents, const char *mostcommonverb){
	std::stringstream sqlcommand;
	sqlcommand << "UPDATE Graphlist SET uniquerelations=" << nounique << ", highestdegreestring='" << highestdelist << "', highestdegree=" << highestdeg << ", largestconnected=" << largconn << ", noconnectedcomponents=" << nocomponents << ", mostcommonverbstring='" << mostcommonverb << "' WHERE key=" << graphid << ";";
	char *zErrMsg;
	bool rc = do_sqlite_exec(sqlcommand.str().c_str(), &zErrMsg);
	if (!rc){
		printf_s("SQL error3: %s\n", zErrMsg);
		free(zErrMsg);
	}
}

void DrInventorSqlitedb::sqlcreate(void){
	char *zErrMsg;
	bool rc = do_sqlite_exec("CREATE TABLE Graphlist(key INTEGER PRIMARY KEY, filename TEXT collate nocase, tknfilename TEXT collate nocase, relationnodes UINT, conceptnodes UINT, uniquerelations UINT, mostcommonverbstring TEXT, highestdegreestring TEXT, highestdegree UINT, noconnectedcomponents UINT, largestconnected UINT);", &zErrMsg);
	if (!rc){
		printf_s("SQL error1: %s\n", zErrMsg);
		free(zErrMsg);
		dbopen = false;
		sqlite3_close(db);
	}
}

void DrInventorSqlitedb::UpdateNodeCount(long graphid, unsigned int relcount, unsigned int conceptcount){
	std::stringstream sqlcommand;
	sqlcommand << "UPDATE Graphlist SET relationnodes=" << relcount << ", conceptnodes=" << conceptcount << " WHERE key=" << graphid << ";";
	char *zErrMsg;
	bool rc = do_sqlite_exec(sqlcommand.str().c_str(), &zErrMsg);
	if (!rc){
		printf_s("SQL error2: %s\n", zErrMsg);
		free(zErrMsg);
	}
}

long DrInventorSqlitedb::GetIDfromLongID(const char *longid){
	std::stringstream sqlcommand;
	sqlcommand << "SELECT key from Graphlist WHERE filename='" << longid << "';";
	std::vector<std::string> values;
	unsigned int nrows, ncols;
	do_sqlite_table(sqlcommand.str().c_str(), &values, &nrows, &ncols);
	if (nrows > 0){
		return atol(values[0].c_str());
	}
	else return -1;
}

int DrInventorSqlitedb::GetTotalNodeCount(const char *longid){
	std::stringstream sqlcommand;
	sqlcommand << "SELECT relationnodes, conceptnodes from Graphlist WHERE filename='" << longid << "';";
	std::vector<std::string> values;
	unsigned int nrows, ncols;
	do_sqlite_table(sqlcommand.str().c_str(), &values, &nrows, &ncols);
	if (nrows > 0){
		return atol(values[0].c_str()) + atol(values[1].c_str());
	}
	else return -1;
}

int DrInventorSqlitedb::GetTotalNodeCount(long neoid){
	std::stringstream sqlcommand;
	sqlcommand << "SELECT relationnodes, conceptnodes from Graphlist WHERE key='" << neoid << "';";
	std::vector<std::string> values;
	unsigned int nrows, ncols;
	do_sqlite_table(sqlcommand.str().c_str(), &values, &nrows, &ncols);
	if (nrows > 0){
		return atol(values[0].c_str()) + atol(values[1].c_str());
	}
	else return -1;
}

long DrInventorSqlitedb::InsertFile(const char *filename, const char *uniqueid, bool *isnew, bool createfolder){
	//This is all temporary storage of files in folder structure. Should be changed in full integration but need to know where exactly
	std::stringstream sqlcommand;
	sqlcommand << "SELECT key from Graphlist WHERE tknfilename='" << uniqueid << "';";
	std::vector<std::string> values;
	unsigned int nrows, ncols;
	do_sqlite_table(sqlcommand.str().c_str(), &values, &nrows, &ncols);
	if (nrows > 0){
		*isnew = false;
		return atol(values[0].c_str());
	}
	sqlcommand.str("");
	sqlcommand << "INSERT into Graphlist(filename,tknfilename) VALUES('" << filename << "','" << uniqueid << "');";
	char *zErrMsg;
	bool rc = do_sqlite_exec(sqlcommand.str().c_str(), &zErrMsg);
	if (!rc){
		printf_s("SQL error2: %s\n", zErrMsg);
		free(zErrMsg);
		return -1;
	}
	*isnew = true;
	//Create the directory to store the files and copy the originals into this directory
	long filekey = (long)sqlite3_last_insert_rowid(db);
	if (createfolder){
		std::stringstream newdir;
		newdir << filehome << filekey;
		#if defined(_WIN32)
		_mkdir(newdir.str().c_str());
		#else 
		mkdir(newdir.str().c_str(), 0777);
		#endif
	}

	/*//Making copies of the CSV files. Probably not really needed
	newdir << fb << filename;
	copyfile(filename, newdir.str().c_str());
	newdir.str("");
	newdir << filehome << filekey << fb << tknfilename;
	copyfile(tknfilename, newdir.str().c_str());*/
	return filekey;
}

bool DrInventorSqlitedb::copyfile(const char *from, const char *to){
	//Platform indepdendent copyfile function
	char buf[8192];
	size_t size;
	FILE* source = fopen(from, "rb");
	FILE* dest = fopen(to, "wb");

	if (source && dest){
		while (size = fread(buf, 1, BUFSIZ, source)) {
			fwrite(buf, 1, size, dest);
		}

		fclose(source);
		fclose(dest);
		return true;
	}
	else
		return false;
}

bool DrInventorSqlitedb::isopen(void){
	return dbopen;
}

std::string DrInventorSqlitedb::GiveFileHome(void){
	return filehome;
}
