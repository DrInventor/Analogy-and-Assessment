// ConsoleApplication2.cpp : Defines the entry point for the console application.
//

#include "ConsoleApplication2.h"

int main(int argc, char** argv)
{
	/*
	The main function. Program should be run with an argument of 
	-pg process one file: which file to process and insert into database. The tknfile should just be filename_tokenInfo_list.csv.
	-dd dump database to file
	-af Process all applicable .csv files
	First the settings file is read and the sqlitedb has to be opened - pointers to this object should be passed
	Curl will also need to be initialised
	*/
	//printf_s("Hello World\n");
	struct settings set;
	parsesettings(&set);
	curl_global_init(CURL_GLOBAL_ALL);
	if (argc > 1){
		if (strcmp(argv[1], "-pg") == 0){
			//Process file into Graph
			if (argc < 3){
				printf_s("Usage -pg \"filename\"\n");
				return 0;
			}
			else{
				DrInventorSqlitedb sqlitedb(set.store.c_str(), set.dbfil.c_str(), slash);
				ProcessGraph(argv[2], &sqlitedb, set.gravz.c_str(), set.neo4j.c_str());
			}
		}
		else if (strcmp(argv[1], "-dd") == 0){
			//Dump database to file
			if (argc < 3){
				printf_s("Usage -dd \"filename\"\n");
				return 0;
			}
			else{
				DrInventorSqlitedb sqlitedb(set.store.c_str(), set.dbfil.c_str(), slash);
				sqlitedb.DumpDatabase(argv[2]);
			}
		}
		else if (strcmp(argv[1], "-af") == 0){
			printf_s("processing all .csv files that have matching _tokenfile.csv\n");
			DrInventorSqlitedb sqlitedb(set.store.c_str(), set.dbfil.c_str(), slash);
			ProcessAllGraphs(&sqlitedb, set.gravz.c_str(), set.neo4j.c_str());
		}
		else if (strcmp(argv[1], "-kr") == 0){
			//Extract range of sentences from the csv file and output a new csv file
			//Usage -kr inputfile outputfile startingSID endingSID
			KeepSentences(argc, argv, true);
		}
		else if (strcmp(argv[1], "-ks") == 0){
			//Extract specific sentences from the csv file and output a new csv file
			//Usage -ks inputfile outputfile sID1 sID2 sID3.......
			KeepSentences(argc, argv, false);
		}
		else if (strcmp(argv[1], "-gc") == 0){
			//Convert GV file
			if (argc < 4){
				printf_s("Usage -gc \"filename\" graphid\n");
				return 0;
			}
			else{
				Neo4jInteract neo4jinteract(set.neo4j.c_str());
				neo4jinteract.GVtoNeo4j(argv[2], atoi(argv[3]));
			}
		}
		else if (strcmp(argv[1], "-tc") == 0 && argc >=2){
			DrInventorSqlitedb sqlitedb(set.store.c_str(), set.dbfil.c_str(), slash);
			ProcessGraph(argv[2], &sqlitedb, set.gravz.c_str(), set.neo4j.c_str(),true);
		}
	}
	else
		printf_s("Need to use options -pg, -dd, gc");
	//printf_s("\nGoodbye Cruel World\n");
	return 0;
}

int KeepSentences(int argc, char** argv, bool range){
	if (range && argc < 6)
		return 1;
	else if (!range && argc < 5)
		return 1;
	std::vector<int> sids;
	int starting = atoi(argv[4]); int ending;
	if (!range){
		for (int i = 4; i < argc; ++i){
			sids.emplace_back(atoi(argv[i]));
		}
	}
	else{
		ending = atoi(argv[5]);
	}
	std::ifstream toread;
	toread.open(argv[2]);
	if (!toread.is_open())
		return 1;
	FILE *out;
	out = fopen(argv[3], "w");
	if (!out)
		return 1;
	std::string temp;
	getline(toread, temp);
	fprintf_s(out, "%s\n", temp.c_str());
	while (getline(toread, temp)){
		int sID;
		std::string leftover;
		leftover = temp.substr(0, temp.find('\t'));
		sscanf_s(leftover.c_str(), "\"%d\"", &sID);
		if (range){
			if (sID >= starting && sID <= ending)
				fprintf_s(out, "%s\n", temp.c_str());
		}
		else{
			if (std::find(sids.begin(), sids.end(), sID) != sids.end()){
				fprintf_s(out, "%s\n", temp.c_str());
			}
		}
	}
	return 2;
}

int ProcessAllGraphs(DrInventorSqlitedb *sqlitedb, const char *graphviz, const char *neo4jlo){
	/*
	Searches current directory for all .csv files
	If the file ends in _tokenInfo_list.csv stored in different array
	Only csv files with a respective _tokenInfo_list are processed
	*/
	tinydir_dir dir;
	tinydir_open(&dir, ".");

	std::vector<std::string> filenames;
	std::vector<std::string> tokens;
	while (dir.has_next)
	{
		tinydir_file file;
		tinydir_readfile(&dir, &file);
		
		if (!file.is_dir && stricmp(file.extension,"csv") == 0)
		{
			if (strstr(file.name, "_tokenInfo_list.csv") != NULL)
				tokens.emplace_back(file.name);
			else filenames.emplace_back(file.name);
		}

		tinydir_next(&dir);
	}

	tinydir_close(&dir);
	for (unsigned int i = 0; i < filenames.size(); ++i){
		char buf2[2048];
		strcpy(buf2, filenames[i].c_str());
		buf2[strlen(buf2) - 4] = '\0';
		strcat(buf2, "_tokenInfo_list.csv");
		if (std::find(tokens.begin(), tokens.end(), buf2) != tokens.end()){
			ProcessGraph(filenames[i].c_str(), sqlitedb, graphviz, neo4jlo);
		}
	}
		
	return 0;
}

int ProcessGraph(const char *argument, DrInventorSqlitedb *sqlitedb, const char *graphviz, const char *neo4jlo,  bool tc){
	/*
	Lots to do
	Currently: It reads the file and creates the ExtractTriples object by passing the filename and tknfile - The initial graphs(plain white) are made
	curl needs to be initialised for the later Neo4j insertions
	It then calls the database function to open the sqlite database, arguments are 1) main directory to store all the files(subdirectories will be created). 2) location of sqlite database file
	The files are inserted into the database and a graphid is received
	MakeNewLinks is called to create the graphs with only nouns / verbs and then it combines the verbs
	GiveTriples creates the triples and then assigns to the 3 vectors the triples of verbs, subjects, objects
	dotriple object no longer needed so destroy
	ROSFromTriples object created with the 3 vectors.doros needs to know the database object so give pointer
	PrintGVFile will struct the graph(makelinks is called when needed) and then outputs the GVfile, it also does the Neo4j insertions
	Finally GraphInterface gets some properties of the graph and then these are stored in the SQL database
	*/
	if (strlen(argument) < 1)
		return 0;
	char buf[512];
	strcpy(buf, argument);
	printf_s("opening %s\n", buf);
	char buf2[2048];
	strcpy(buf2, buf);
	buf2[strlen(buf2) - 4] = '\0';
	strcat(buf2, "_tokenInfo_list.csv");
	DrInventorFindTriples dotriple(buf, buf2);
	if (!dotriple.isopen()){
		printf_s("Error opening file %s", buf);
		return 0;
	}
	dotriple.MakeNewLinks(tc);
	buf[strlen(buf) - 4] = '\0';
	bool isnew;
	long gid = sqlitedb->InsertFile(buf, buf2, &isnew);
	if (gid == -1){
		printf_s("Error with inserting file to database\n");
		return 0;
	}
	sprintf_s(buf2, "%s%ld%c%s.gv", sqlitedb->GiveFileHome().c_str(),gid,slash,buf);
	std::vector<std::string> vb, sbj, obj;
	std::vector<long> sentid;
	//dotriple.GiveTriples(&vb, &sbj, &obj, &sentid);
	dotriple.MakeTripleSent();
	if (tc){
		dotriple.PrintInbetweenGraphs();
		dotriple.WriteTriplesToCSVFile("triplecsv.csv");
		return 0;
	}
	Neo4jInteract neo4db(neo4jlo);
	if (neo4db.isopen() && !isnew){
		std::stringstream cyphercmd;
		cyphercmd << "MATCH (n) WHERE n.Graphid=" << gid << " OPTIONAL MATCH (n)-[r]-() DELETE n,r;";
		neo4db.neo4cypher(cyphercmd.str().c_str());
	}
	//ROSFromTriples doros(vb, sbj, obj, sentid, gid);
	ROSFromTriples doros(&dotriple.triplesent, gid);
	doros.gvlocation = graphviz;
	doros.sqlitedb = sqlitedb;
	doros.neodb = &neo4db;
	std::stringstream writetodir;
	writetodir << sqlitedb->GiveFileHome() << gid << slash;
	printf_s("Graphid : %d\n", gid);
	doros.PrintGVFile(buf2, true, neo4db.isopen(), writetodir.str().c_str());
	
	GraphInterface testing(&doros.concepts, &doros.relations, &doros.links, gid);
	std::stringstream highestdegstring, mostcommonverbstring;
	for (unsigned int i = 0; i < testing.highestdegreestring.size(); ++i){
		highestdegstring << "''" << testing.highestdegreestring[i] << "''";
		if (i != testing.highestdegreestring.size() - 1)
			highestdegstring << ";";
	}
	for (unsigned int i = 0; i < testing.mostcommonverb.size(); ++i){
		mostcommonverbstring << "''" << testing.mostcommonverb[i] << "''";
		if (i != testing.mostcommonverb.size() - 1)
			mostcommonverbstring << ";";
	}
	sqlitedb->UpdateProperties(gid, testing.nouniquerelat, highestdegstring.str().c_str(), testing.hghdeg, testing.lrgcomp, testing.nocomponents, mostcommonverbstring.str().c_str());
	return 0;
}

bool parsesettings(struct settings *aset){
	std::ifstream toread;
	toread.open("settings.txt");
	if (toread.is_open()){
		std::string temp[2];
		while (getline(toread, temp[0])){
			size_t pos;
			if ((pos = temp[0].find('=')) != std::string::npos){
				temp[1] = temp[0].substr(0, pos);
				temp[0] = temp[0].substr(pos + 1);
				if (temp[1] == "neo4j")
					aset->neo4j = temp[0];
				else if (temp[1] == "gravz")
					aset->gravz = temp[0];
				else if (temp[1] == "store")
					aset->store = temp[0];
				else if (temp[1] == "dbfil")
					aset->dbfil = temp[0];
			}
		}
		toread.close();
	}
	if (aset->neo4j == "")
		aset->neo4j = "http://localhost:7474/";
	if (aset->dbfil == ""){
		char buf[128];
		sprintf_s(buf, "results%cdata.db", slash);
		aset->dbfil = buf;
	}
	if (aset->store == ""){
		char buf[64];
		sprintf_s(buf, "results%c", slash);
		aset->store = buf;
	}
	size_t found;
	//Ensure /r has not been left in string (Windows -> Linux settings)
	if ((found = aset->neo4j.find('\r')) != std::string::npos)
		aset->neo4j = aset->neo4j.substr(0, found);
	if ((found = aset->dbfil.find('\r')) != std::string::npos)
		aset->dbfil = aset->dbfil.substr(0, found);
	if ((found = aset->gravz.find('\r')) != std::string::npos)
		aset->gravz = aset->gravz.substr(0, found);
	if ((found = aset->store.find('\r')) != std::string::npos)
		aset->store = aset->store.substr(0, found);
	return true;
}


//Some testing functions
/*std::vector<std::string> splitbydelimiter(std::string tosplit, std::string delim, bool usequotes){
	std::string leftover = tosplit;
	size_t found;
	std::vector<std::string> toreturn;
	printf_s("processing: %s\n", tosplit.c_str());
	if (!usequotes){
		while ((found = leftover.find(delim)) != std::string::npos){
			toreturn.emplace_back(leftover.substr(0, found));
			leftover = leftover.substr(found + delim.size());
		}
	}
	else{
		while ((found = leftover.find(delim)) != std::string::npos){
			if (leftover.at(0) == '\"'){
				std::string buf = "\"";
				buf += delim;
				bool keepgoing = true;
				bool rplinstr = false;
				found = leftover.find(buf);
				//add in ignore \",
				while (keepgoing){
					if (found != std::string::npos && found > 0 && leftover.at(found - 1) == '\\'){
						rplinstr = true;
						found = leftover.find(buf, found + 1);
					}
					else keepgoing = false;
				}
				if (found != std::string::npos)
					toreturn.emplace_back(leftover.substr(1, found - 1));
				else{
					found = leftover.find("\"", 1);
					if (found != std::string::npos)
						toreturn.emplace_back(leftover.substr(1, found - 1));
					else{
						toreturn.emplace_back(leftover);
						found = leftover.size();
					}
				}
				if (found + buf.size() < leftover.size())
					leftover = leftover.substr(found + buf.size());
				else
					leftover = "";
				replaceinstring(*(toreturn.end() - 1), "\\\"", "\"");
			}
			else{
				toreturn.emplace_back(leftover.substr(0, found));
				leftover = leftover.substr(found + delim.size());
			}
		}
	}
	if (leftover.size() > 0)
		toreturn.emplace_back(leftover);
	return toreturn;
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream){
	size_t written;
	written = fwrite(ptr, size, nmemb, stream);
	return written;
}

void replaceinstring(std::string &toedit, const char *rpl, const char *with){
	size_t found;
	while ((found = toedit.find(rpl)) != std::string::npos){
		toedit.replace(found, strlen(rpl), with);
	}
}*/

/*bool libcurlSaveFile(const wchar_t *url, const wchar_t *filename){
	char *urlc = new char[wcslen(url) + 2];
	size_t conv;
	wcstombs(&conv, urlc, wcslen(url) + 2, url, wcslen(url) + 1);
	CURL *curl;
	FILE *fp;
	CURLcode res;
	curl = curl_easy_init();
	if (curl) {
		_wfopen(&fp, filename, L"wb");
		if (fp){
			curl_easy_setopt(curl, CURLOPT_URL, urlc);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
			res = curl_easy_perform(curl);
			curl_easy_cleanup(curl);
			fclose(fp);
			delete[] urlc;
			return true;
		}
		else {
			delete[] urlc;
			return false;
		}
	}
	else{
		delete[] urlc;
		return false;
	}
}*/

/*std::string computeMD5(const char *filename){
	const size_t BufferSize = 144 * 7 * 1024; 
	char* buffer = new char[BufferSize]; 
	std::ifstream file; 
	std::istream* input = NULL; 
	MD5 digestMd5;
	file.open(filename, std::ios::in | std::ios::binary); 
	if (!file) { 
		std::cerr << "Can't open '" << filename << "'" << std::endl; return ""; 
	} 
	input = &file; 
	while (*input) { 
		input->read(buffer, BufferSize); 
		std::size_t numBytesRead = size_t(input->gcount()); 
		digestMd5.add(buffer, numBytesRead); 
	} 
	file.close(); 
	delete[] buffer;

	return digestMd5.getHash();
}*/
//End testing functions







