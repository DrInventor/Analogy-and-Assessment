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
	DrInventorSqlitedb sqlitedb(set.store.c_str(), set.dbfil.c_str());
	curl_global_init(CURL_GLOBAL_ALL);
	if (argc > 1){
		if (stricmp(argv[1], "-pg") == 0){
			//Process file into Graph
			if (argc < 3){
				printf_s("Usage -pg \"filename\"\n");
				return 0;
			}
			else 
				ProcessGraph(argv[2], &sqlitedb, set.gravz.c_str(), set.neo4j.c_str());
		}
		else if (stricmp(argv[1], "-dd") == 0){
			//Dump database to file
			if (argc < 3){
				printf_s("Usage -dd \"filename\"\n");
				return 0;
			}
			else
				sqlitedb.DumpDatabase(argv[2]);
		}
		else if (stricmp(argv[1], "-af") == 0){
			printf_s("processing all .csv files that have matching _tokenfile.csv\n");
			ProcessAllGraphs(&sqlitedb, set.gravz.c_str(), set.neo4j.c_str());
		}
	}
	else
		printf_s("Need to use options -pg, -dd");
	//printf_s("\nGoodbye Cruel World\n");
	return 0;
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
		strcpy_s(buf2, filenames[i].c_str());
		buf2[strlen(buf2) - 4] = '\0';
		strcat_s(buf2, "_tokenInfo_list.csv");
		if (std::find(tokens.begin(), tokens.end(), buf2) != tokens.end())
			ProcessGraph(filenames[i].c_str(), sqlitedb, graphviz, neo4jlo);
	}
		
	return 0;
}

int ProcessGraph(const char *argument, DrInventorSqlitedb *sqlitedb, const char *graphviz, const char *neo4jlo){
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
	strcpy_s(buf, argument);
	printf_s("opening %s\n", buf);
	char buf2[2048];
	strcpy_s(buf2, buf);
	buf2[strlen(buf2) - 4] = '\0';
	strcat_s(buf2, "_tokenInfo_list.csv");
	DrInventorFindTriples dotriple(buf, buf2);
	if (!dotriple.isopen()){
		printf_s("Error opening file %s", buf);
		return 0;
	}
	long gid = sqlitedb->InsertFile(buf, buf2);
	dotriple.MakeNewLinks();
	buf[strlen(buf) - 4] = '\0';
	sprintf_s(buf2, "%s%ld\\%s.gv", sqlitedb->GiveFileHome().c_str(),gid,buf);
	std::vector<std::string> vb, sbj, obj;
	dotriple.GiveTriples(&vb, &sbj, &obj);
	//dotriple.PrintInbetweenGraphs();
	//dotriple.WriteTriplesToCSVFile("triplecsv.csv");
	dotriple.~DrInventorFindTriples();
	ROSFromTriples doros(vb, sbj, obj, gid);
	doros.gvlocation = graphviz;
	doros.sqlitedb = sqlitedb;
	Neo4jInteract neo4db(neo4jlo);
	doros.neodb = &neo4db;
	std::stringstream writetodir;
	writetodir << sqlitedb->GiveFileHome() << gid << "\\";
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
	if (aset->dbfil == "")
		aset->dbfil = "results\\data.db";
	if (aset->store == "")
		aset->store = "results\\";
	return true;
}



void replaceinstring(std::string &toedit, const char *rpl, const char *with){
	size_t found;
	while ((found = toedit.find(rpl)) != std::string::npos){
		toedit.replace(found, strlen(rpl), with);
	}
}

//Some testing functions
std::vector<std::string> splitbydelimiter(std::string tosplit, std::string delim, bool usequotes){
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

/*bool libcurlSaveFile(const wchar_t *url, const wchar_t *filename){
	char *urlc = new char[wcslen(url) + 2];
	size_t conv;
	wcstombs_s(&conv, urlc, wcslen(url) + 2, url, wcslen(url) + 1);
	CURL *curl;
	FILE *fp;
	CURLcode res;
	curl = curl_easy_init();
	if (curl) {
		_wfopen_s(&fp, filename, L"wb");
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







