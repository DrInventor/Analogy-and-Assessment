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
	printf_s("GenerateROS Version 0.2.0\n");
	if (argc > 1){
		if (strcmp(argv[1], "-pg") == 0){
			//Process file into Graph
			if (argc < 4){
				printf_s("Usage -pg \"filename\" \"UniqueID\"\n");
				return 0;
			}
			else{
				bool dogv = false;
				if (argc > 4 && strcmp(argv[4], "-gv") == 0)
					dogv = true;
				DrInventorSqlitedb sqlitedb(set.store.c_str(), set.dbfil.c_str(), slash);
				Neo4jInteract neo4db(set.neo4j.c_str(), set.neouser.c_str(), set.neopass.c_str());
				ProcessGraph(argv[2], &sqlitedb, set.gravz.c_str(), &neo4db, dogv, argv[3], false);
			}
		}
		else if (strcmp(argv[1], "-po") == 0){
			if (argc < 4){
				printf_s("Usage -po \"filename\" \"UniqueID\"\n");
				return 0;
			}
			else{
				bool dogv = false;
				if (argc > 4 && strcmp(argv[4], "-gv") == 0)
					dogv = true;
				DrInventorSqlitedb sqlitedb(set.store.c_str(), set.dbfil.c_str(), slash);
				Neo4jInteract neo4db(set.neo4j.c_str(), set.neouser.c_str(), set.neopass.c_str());
				ProcessGraph(argv[2], &sqlitedb, set.gravz.c_str(), &neo4db, dogv, argv[3], true);
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
			bool dogv = false;
			if (argc > 2 && strcmp(argv[2], "-gv") == 0)
				dogv = true;
			DrInventorSqlitedb sqlitedb(set.store.c_str(), set.dbfil.c_str(), slash);
			ProcessAllGraphs(&sqlitedb, set.gravz.c_str(), set.neo4j.c_str(),dogv, set.neouser.c_str(), set.neopass.c_str());
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
				Neo4jInteract neo4jinteract(set.neo4j.c_str(), set.neouser.c_str(), set.neopass.c_str());
				neo4jinteract.GVtoNeo4j(argv[2], atoi(argv[3]));
			}
		}
		else if (strcmp(argv[1], "-tc") == 0 && argc >=2){
			/*DrInventorSqlitedb sqlitedb(set.store.c_str(), set.dbfil.c_str(), slash);
			ProcessGraph(argv[2], &sqlitedb, set.gravz.c_str(), set.neo4j.c_str(),true);*/
			TestingStuff(&set);
		}
		else if (strcmp(argv[1], "-cg") == 0 && argc >= 3){
			//GraphTesting(argv);
		}
	}
	else
		printf_s("Need to use options -pg, -dd, gc");
	return 0;
}

void TestingStuff(struct settings *set){
/*	Neo4jInteract neodb(set->neo4j.c_str());
	struct agraph thegraph = neodb.GetGraphbyID(1);
	printf_s("%d %d %d \n", thegraph.concepts.size(), thegraph.relations.size(), thegraph.links.size());
	for (unsigned int i = 0; i < thegraph.links.size(); ++i){
		printf_s("%d %d %d\n", thegraph.links[i].sbj, thegraph.links[i].obj, thegraph.links[i].vb);
	}
	for (unsigned int i = 0; i < thegraph.concepts.size(); ++i){
		printf_s("%s %d\n", thegraph.concepts[i].word.c_str(), thegraph.concepts[i].id);
	}
	for (unsigned int i = 0; i < thegraph.relations.size(); ++i){
		printf_s("%s %d\n", thegraph.relations[i].word.c_str(), thegraph.relations[i].id);
	}*/
	DrInventorSqlitedb *db;
	db = new DrInventorSqlitedb(set->dbfil.c_str());
	int count = db->GetTotalNodeCount(6);
	printf_s("AB : %d\n", count);
	delete db;
	GraphProperties gphprop(18, set->neo4j.c_str(), set->neouser.c_str(), set->neopass.c_str());
	if (gphprop.is_open())
		gphprop.ConvertGraphtoNodes();
	/*std::vector<int> testing;
	for (unsigned int i = 0; i < 20; ++i)
		testing.emplace_back(i);
	printf_s("FIND : %d\n", std::find(testing.begin(), testing.end(), 10)-testing.begin());*/
	/*std::string response;
	neodb.neo4cypher("Match n where n.Graphid=1 return id(n);", &response);
	printf_s(response.c_str());*/
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

int ProcessAllGraphs(DrInventorSqlitedb *sqlitedb, const char *graphviz, const char *neo4jlo, bool dogv, const char *neouser, const char *neopass){
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
	Neo4jInteract neo4db(neo4jlo, neouser, neopass);;
	for (unsigned int i = 0; i < filenames.size(); ++i){
		char buf2[2048];
		strcpy(buf2, filenames[i].c_str());
		buf2[strlen(buf2) - 4] = '\0';
		strcat(buf2, "_tokenInfo_list.csv");
		if (std::find(tokens.begin(), tokens.end(), buf2) != tokens.end()){
			ProcessGraph(filenames[i].c_str(), sqlitedb, graphviz, &neo4db, dogv, buf2, true);
		}
	}
		
	return 0;
}

long ProcessGraphWrap(const char *argument, const char *longid){
	//Mostly the Same as ProcessGraph but tuned for the JavaWrapper arguments
	if (strlen(argument) < 1)
		return 0;
	struct settings set;
	parsesettings(&set);
	curl_global_init(CURL_GLOBAL_ALL);
	DrInventorSqlitedb sqlitedb(set.store.c_str(), set.dbfil.c_str(), slash);
	Neo4jInteract neo4db(set.neo4j.c_str(), set.neouser.c_str(), set.neopass.c_str());
	if (!neo4db.isopen()){
		printf_s("\nNot making GV files and cannot access Neo4j database, nothing to do: Exiting\n");
		return 0;
	}
	DrInventorFindTriples dotriple(argument);
	if (!dotriple.isopen()){
		printf_s("Error opening file %s", argument);
		return 0;
	}
	dotriple.MakeNewLinks(false);
	bool isnew;
	long gid = sqlitedb.InsertFile(argument, longid, &isnew, false);
	if (gid == -1){
		printf_s("Error with inserting file to database\n");
		return 0;
	}
	dotriple.MakeTripleSent();
	if (neo4db.isopen() && !isnew){
		std::stringstream cyphercmd;
		cyphercmd << "MATCH (n) WHERE n.Graphid=" << gid << " OPTIONAL MATCH (n)-[r]-() DELETE n,r;";
		neo4db.neo4cypher(cyphercmd.str().c_str());
	}
	ROSFromTriples doros(&dotriple.triplesent, gid);
	doros.gvlocation = "";
	doros.sqlitedb = &sqlitedb;
	doros.neodb = &neo4db;
	
	doros.InserttoNeo4j();

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
	sqlitedb.UpdateProperties(gid, testing.nouniquerelat, highestdegstring.str().c_str(), testing.hghdeg, testing.lrgcomp, testing.nocomponents, mostcommonverbstring.str().c_str());
	return gid;
}

int ProcessGraph(const char *argument, DrInventorSqlitedb *sqlitedb, const char *graphviz, Neo4jInteract *neodb,  bool dogv, std::string longid, bool oldtkn, bool tc){
	/*
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
	if (oldtkn){
		strcpy(buf2, buf);
		buf2[strlen(buf2) - 4] = '\0';
		strcat(buf2, "_tokenInfo_list.csv");
	}
	if (!dogv && !neodb->isopen()){
		printf_s("\nNot making GV files and cannot access Neo4j database, nothing to do: Exiting\n");
		return 0;
	}
	DrInventorFindTriples *dotriple;
	if (oldtkn)
		dotriple = new DrInventorFindTriples(buf, buf2);
	else
		dotriple = new DrInventorFindTriples(buf);
	if (!dotriple->isopen()){
		printf_s("Error opening file %s", buf);
		return 0;
	}
	dotriple->MakeNewLinks(tc);
	buf[strlen(buf) - 4] = '\0';
	bool isnew;
	long gid = sqlitedb->InsertFile(buf, longid.c_str(), &isnew, dogv);

	if (gid == -1){
		printf_s("Error with inserting file to database\n");
		delete dotriple;
		return 0;
	}
	sprintf_s(buf2, "%s%ld%c%s.gv", sqlitedb->GiveFileHome().c_str(),gid,slash,buf);
	
	dotriple->MakeTripleSent();
	if (tc){
		dotriple->PrintInbetweenGraphs();
		dotriple->WriteTriplesToCSVFile("triplecsv.csv");
		delete dotriple;
		return 0;
	}
	if (neodb->isopen() && !isnew){
		std::stringstream cyphercmd;
		cyphercmd << "MATCH (n) WHERE n.Graphid=" << gid << " OPTIONAL MATCH (n)-[r]-() DELETE n,r;";
		neodb->neo4cypher(cyphercmd.str().c_str());
	}
	ROSFromTriples doros(&dotriple->triplesent, gid);
	doros.gvlocation = graphviz;
	doros.sqlitedb = sqlitedb;
	doros.neodb = neodb;
	std::stringstream writetodir;
	writetodir << sqlitedb->GiveFileHome() << gid << slash;
	printf_s("Graphid : %d\n", gid);
	if (dogv)
		doros.PrintGVFile(buf2, true, neodb->isopen(), writetodir.str().c_str());
	else
		doros.InserttoNeo4j();
	
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
	delete dotriple;
	return gid;
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
				else if (temp[1] == "neouser")
					aset->neouser = temp[0];
				else if (temp[1] == "neopass")
					aset->neopass = temp[0];
			}
		}
		toread.close();
	}
	if (aset->neo4j == "")
		aset->neo4j = "http://localhost:7474/";
	if (aset->dbfil == ""){
		char buf[128];
		sprintf_s(buf, "data.db", slash);
		aset->dbfil = buf;
	}
	if (aset->store == ""){
		char buf[64];
		sprintf_s(buf, "results%c", slash);
		aset->store = buf;
	}
	size_t found;
	//Ensure \r has not been left in string (Windows -> Linux settings)
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
std::vector<std::string> splitbydelimiter(std::string tosplit, std::string delim, bool usequotes){
	std::string leftover = tosplit;
	size_t found;
	std::vector<std::string> toreturn;
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
	if (leftover.size() > 0){
		if (!usequotes)
			toreturn.emplace_back(leftover);
		else{
			int alter;
			alter = 0;
			if (leftover.at(0) == '\"' && leftover.at(leftover.size() - 1) == '\"')
				alter = 1;
			toreturn.emplace_back(leftover.substr(0+alter, leftover.size()-(2*alter)));
			leftover = leftover.substr(found + delim.size());
		}
	}
	return toreturn;
}

void replaceinstring(std::string &toedit, const char *rpl, const char *with){
	size_t found;
	while ((found = toedit.find(rpl)) != std::string::npos){
		toedit.replace(found, strlen(rpl), with);
	}
}



