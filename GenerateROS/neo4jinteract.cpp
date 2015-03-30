#include "neo4jinteract.h"

void Neo4jInteract::BeginBatchOperations(int howmuch){
	//Needs to be called before adding any nodes or links to the temporary lists. This ensures enough memory is allocated.
	anode.resize(howmuch);
	anodenest.resize(howmuch);
	nodelabels.resize(howmuch);
	fulllist = new rapidjson::Document();
	fulllinks = new rapidjson::Document();
	fulllist->SetArray();
	fulllinks->SetArray();
}

void Neo4jInteract::EndBatchOperations(void){
	//This is called after ExecuteLinkList, ensure ExecuteLinkList is called AFTER ExecuteNodeList
	//Frees all temporary memory needed for building the JSON file to send to Neo4j REST
	delete fulllist;
	delete fulllinks;
	anode.clear();
	anodenest.clear();
	nodelabels.clear();
	std::vector<rapidjson::Document>().swap(anode);
	std::vector<rapidjson::Document>().swap(anodenest);
	std::vector<rapidjson::Document>().swap(nodelabels);

}

bool Neo4jInteract::ExecuteLinkList(void){
	//Actually add all the links between nodes in one query to the database
	if (!curlok){
		EndBatchOperations();
		return false;
	}
	struct MemoryStruct chunk;

	rapidjson::StringBuffer strbuf;
	rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
	fulllinks->Accept(writer);

	CURL *curl;
	CURLcode res;
	curl = curl_easy_init();
	if (!curl){
		EndBatchOperations();
		return false;
	}
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, "charsets: utf-8");
	headers = curl_slist_append(headers, authenttoken.c_str());
	std::stringstream neo4jurl;
	neo4jurl << neo4jlocation << "db/data/batch";
	curl_easy_setopt(curl, CURLOPT_URL, neo4jurl.str().c_str());
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strbuf.GetString());
	res = curl_easy_perform(curl);
	long http_code = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
	if (res != CURLE_OK){
		printf_s("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		curl_easy_cleanup(curl);
		free(chunk.memory);
		EndBatchOperations();
		return false;
	}
	curl_easy_cleanup(curl);
	free(chunk.memory);
	EndBatchOperations();
	return true;

}

std::vector<long> Neo4jInteract::ExecuteNodeList(void){
	//Actually add all the nodes in the temporary list
	std::vector<long> toreturn;
	if (!curlok)
		return toreturn;
	rapidjson::StringBuffer strbuf;
	rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
	fulllist->Accept(writer);
	struct MemoryStruct chunk;
	CURL *curl;
	CURLcode res;
	curl = curl_easy_init();
	if (curl){
		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Accept: application/json");
		headers = curl_slist_append(headers, "Content-Type: application/json");
		headers = curl_slist_append(headers, "charsets: utf-8");
		headers = curl_slist_append(headers, authenttoken.c_str());
		std::stringstream neo4jurl;
		neo4jurl << neo4jlocation << "db/data/batch";
		curl_easy_setopt(curl, CURLOPT_URL, neo4jurl.str().c_str());
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strbuf.GetString());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
		res = curl_easy_perform(curl);
		long http_code = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
		if (res != CURLE_OK){
			printf_s("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			curl_easy_cleanup(curl);
			free(chunk.memory);
			return toreturn;
		}
		toreturn.resize(anode.size());
		rapidjson::Document ret;
		ret.Parse(chunk.memory);
		if (ret.IsArray()){
			for (unsigned int i = 0; i < ret.Size(); i++){
				if (ret[i].IsObject() && ret[i].HasMember("id")){
					int which = ret[i]["id"].GetInt64();
					if (ret[i]["body"].HasMember("metadata") && ret[i]["body"]["metadata"].IsObject()){
						rapidjson::StringBuffer sb;
						rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
						ret[i]["body"]["metadata"].Accept(writer);
						rapidjson::Document metadata;
						metadata.Parse(sb.GetString());
						if (metadata.HasMember("id")){
							if (metadata["id"].IsInt64()){
								toreturn[which] = metadata["id"].GetInt64();
							}
						}
					}
				}
			}
		}
		free(chunk.memory);
	}
	return toreturn;
}

bool Neo4jInteract::AddNodetoList(int which, const char *word, int type, const char *gvlab, long graphid, long sentid){
	//This adds a node to the temporary list of nodes that are being prepared to send as one query to the Neo4j database
	if (anode.size() == 0)
		return false;
	anode[which].SetObject();
	rapidjson::Document::AllocatorType&allocator = anode[which].GetAllocator();
	rapidjson::Value valObjectString(rapidjson::kStringType);
	valObjectString.SetString(word, allocator);
	anode[which].AddMember("method", "POST", allocator);
	anode[which].AddMember("id", which, allocator);
	anode[which].AddMember("to", "/node", allocator);

	anodenest[which].SetObject();
	rapidjson::Document::AllocatorType&nestallocator = anodenest[which].GetAllocator();
	anodenest[which].AddMember("Word", valObjectString, nestallocator);
	anodenest[which].AddMember("Graphid", graphid, nestallocator);
	valObjectString.SetString(gvlab, nestallocator);
	anodenest[which].AddMember("GVLabel", valObjectString, nestallocator);
	if (type == 2){
		anodenest[which].AddMember("SentID", sentid, nestallocator);
	}
	anode[which].AddMember("body", anodenest[which], allocator);
	rapidjson::Document::AllocatorType&fullallocator = fulllist->GetAllocator();
	fulllist->PushBack(anode[which], fullallocator);
	nodelabels[which].SetObject();
	rapidjson::Document::AllocatorType&eallocator = nodelabels[which].GetAllocator();
	nodelabels[which].AddMember("method", "POST", eallocator);
	char buf[128];
	sprintf_s(buf, "{%d}/labels", which);
	valObjectString.SetString(buf, eallocator);
	nodelabels[which].AddMember("to", valObjectString, eallocator);
	if (type == 1)
		nodelabels[which].AddMember("body", "Noun", eallocator);
	else if (type == 2)
		nodelabels[which].AddMember("body", "Verb", eallocator);
	fulllist->PushBack(nodelabels[which], fullallocator);
	return true;
}

bool Neo4jInteract::AddNodetoNeo4j(const char *word, int type, const char *gvlab, long *nodeid, long graphid, long sentid){
	/*
	Adds a single node to the Neo4j database. Uses rapidjson to form the json expression (could easily form without rapidjson but used anyway)
	Sends information through libcurl
	Neo4j database location needed, read from settings file
	After inserting the node, the returned response will be in json format and this is parsed by rapidjson (needed!)
	From this parsing nodeid is assigned the id given by Neo4j
	Finally this node is then given the label "Noun" or "Verb"
	*/

	CURL *curl;
	CURLcode res;
	rapidjson::Document d;
	d.SetObject();
	rapidjson::Document::AllocatorType&allocator = d.GetAllocator();
	rapidjson::Value valObjectString(rapidjson::kStringType);
	valObjectString.SetString(word, allocator);
	d.AddMember("Word", valObjectString, allocator);
	d.AddMember("Graphid", graphid, allocator);
	valObjectString.SetString(gvlab, allocator);
	d.AddMember("GVLabel", valObjectString, allocator);
	if (type == 2){
		d.AddMember("SentID", sentid, allocator);
	}
	rapidjson::StringBuffer strbuf;
	rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
	d.Accept(writer);

	struct MemoryStruct chunk;

	//const char *thejson = strbuf.GetString();
	if (!curlok){
		*nodeid = -1;
		return false;
	}
	curl = curl_easy_init();
	if (curl){
		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Accept: application/json");
		headers = curl_slist_append(headers, "Content-Type: application/json");
		headers = curl_slist_append(headers, "charsets: utf-8");
		headers = curl_slist_append(headers, authenttoken.c_str());
		std::stringstream neo4jurl;
		neo4jurl << neo4jlocation << "db/data/node";
		curl_easy_setopt(curl, CURLOPT_URL, neo4jurl.str().c_str());
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strbuf.GetString());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
		res = curl_easy_perform(curl);
		long http_code = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
		if (res != CURLE_OK){
			printf_s("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			curl_easy_cleanup(curl);
			free(chunk.memory);
			*nodeid = -1;
			return false;
		}
		rapidjson::Document ret;
		ret.Parse(chunk.memory);
		if (ret.HasMember("metadata")){
			if (ret["metadata"].IsObject()){
				rapidjson::StringBuffer sb;
				rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
				ret["metadata"].Accept(writer);
				rapidjson::Document metadata;
				metadata.Parse(sb.GetString());
				if (metadata.HasMember("id")){
					if (metadata["id"].IsInt64()){
						long theid = metadata["id"].GetInt64();
						*nodeid = theid;
						std::stringstream labelupdate;
						labelupdate << neo4jlocation << "db/data/node/" << theid << "/labels";
						curl_easy_setopt(curl, CURLOPT_URL, labelupdate.str().c_str());
						curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
						curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
						if (type == 1)
							curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "\"Noun\"");
						else if (type == 2)
							curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "\"Verb\"");
						struct MemoryStruct chunk2;
						curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
						curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk2);
						res = curl_easy_perform(curl);
						http_code = 0;
						curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
						if (res != CURLE_OK){
							printf_s("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
							free(chunk.memory);
							free(chunk2.memory);
							curl_easy_cleanup(curl);
							return false;
						}
						curl_easy_cleanup(curl);
						/*for (unsigned int i = 0; i < chunk2.size; ++i)
						printf_s("%c", chunk2.memory[i]);*/
						free(chunk2.memory);
					}
				}
			}
		}
		free(chunk.memory);
		//curl_easy_cleanup(curl);
		return true;
	}
	else{
		free(chunk.memory);
		*nodeid = -1;
		return false;
	}
}

Neo4jInteract::Neo4jInteract(const char *location, const char *user, const char *password){
	/*
	Constructor
	Tests if Curl can make any connection with the database
	If it can't, all other database operations can't be done
	If the program requires Neo4j to continue, then check the Neo4jInteract.curlok variable and if false, exit (or destroy the current object and create a new one later)
	*/
	neo4jlocation = location;
	curl_global_init(CURL_GLOBAL_ALL);
	CURL *curl;
	CURLcode res;
	curlok = true;
	curl = curl_easy_init();
	if (!curl)
		curlok = false;
	else{
		struct MemoryStruct chunk;

		curl_easy_setopt(curl, CURLOPT_URL, neo4jlocation.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
		res = curl_easy_perform(curl);
		long http_code = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
		if (res != CURLE_OK){
			//printf_s("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			curlok = false;
		}
		free(chunk.memory);
	//	curl_easy_cleanup(curl);
	}
	//Authentication
	if (curlok){
		/*CURL *curl;
		curl = curl_easy_init();*/
		struct MemoryStruct chunk;
		std::string newurl = neo4jlocation + "user/" + user;
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Accept: application/json");
		headers = curl_slist_append(headers, "Content-Type: application/json");
		headers = curl_slist_append(headers, "charsets: utf-8");

		if (strlen(user) > 0 && strlen(password) > 0){
			std::stringstream auth;
			auth << user << ":" << password;
			std::string encode = base64_encode(auth.str().c_str(), auth.str().size());
			auth.str("");
			auth << "Authorization: Basic " << encode;
			authenttoken = auth.str().c_str();
			headers = curl_slist_append(headers, authenttoken.c_str());
		}
		else{
			authenttoken = "";
			newurl = neo4jlocation + "db/data/";
		}
		curl_easy_setopt(curl, CURLOPT_URL, newurl.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

		long http_code = 0;
		res = curl_easy_perform(curl);
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
		if (res != CURLE_OK){
			//printf_s("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			curlok = false;
		}
		rapidjson::Document d;
		d.Parse(chunk.memory);
		if (authenttoken != "" && d.HasMember("username") && strcmp(d["username"].GetString(), user) == 0 && d.HasMember("password_change_required")){
			if (d["password_change_required"].GetBool()){
				printf_s("Failed to connect to Neo4j database: Authentication error: Must change password\n");
				curlok = false;
			}
		}
		else if (authenttoken == "" && !d.HasMember("errors"));
		else{
			printf_s("Failed to connect to Neo4j database: Authentication error.\n");
			curlok = false;
		}
		free(chunk.memory);
		curl_easy_cleanup(curl);
	}
}

bool Neo4jInteract::GVtoNeo4j(const char *filename, long graphid){
	std::ifstream toread;
	if (graphid < 1){
		printf_s("Bad Graph ID\n");
		return false;
	}
	if (!curlok){
		printf_s("Cannot connect to Neo4j database\n");
		return false;
	}
	toread.open(filename);
	if (!toread.is_open()){
		printf_s("Could not open file\n");
		return false;
	}
	printf_s("Opening file %s to convert to Neo4j with GraphID: %d\n", filename, graphid);
	std::string temp,leftover;
	getline(toread, temp);
	std::vector<struct anode> conceptnodes;
	std::vector<struct anode> relationnodes;
	std::vector<struct link> links;
	struct anode tempnode;
	struct link templink;
	size_t found;
	//Extract the nodes and links from the GV into vectors
	while (getline(toread, temp)){
		//Remove all tab characters, then remove all whitespace from the start of the line
		replaceinstring(temp, "\t", "");
		int count = 0;
		for (unsigned int i = 0; i < temp.size(); ++i){
			if (temp.at(i) != ' '){
				count = i;
				break;
			}
		}
		temp = temp.substr(count);
		if (temp.at(0) == 'c'){ //concept node
			found = temp.find('[');
			if (found != std::string::npos){
				leftover = temp.substr(1, found - 1);
				tempnode.nodeid = atoi(leftover.c_str());
				found = temp.find("label=\"");
				if (found != std::string::npos){
					tempnode.word = temp.substr(found + 7, temp.find('\"', found+7) - found - 7);
					conceptnodes.emplace_back(tempnode);
				}
			}
		}
		else if (temp.at(0) == 'r'){ //relation node
			//Remove all whitespace to ensure c->r->c structure is correct
			replaceinstring(temp, " ", "");
			found = temp.find('[');
			if (found != std::string::npos){
				leftover = temp.substr(1, found - 1);
				tempnode.nodeid = atoi(leftover.c_str());
				found = temp.find("label=\"");
				if (found != std::string::npos){
					tempnode.word = temp.substr(found + 7, temp.find('\"', found + 7) - found - 7);
					leftover = temp.substr(temp.find(';')+1);
					sscanf_s(leftover.c_str(), "c%d->r%d->c%d", &templink.sbj, &templink.vb, &templink.obj);
					if ((found = temp.find("SentID=")) == std::string::npos)
						tempnode.sentid = 0;
					else{
						leftover = temp.substr(found + 7);
						sscanf_s(leftover.c_str(), "%d", &tempnode.sentid);
					}
					relationnodes.emplace_back(tempnode);
					links.emplace_back(templink);
				}
			}
		}//else ignore line
	}
	toread.close();
	//Now actually put them into the database
	if (isopen()){
		//First remove the old nodes with the matching GraphID
		std::stringstream cyphercmd;
		cyphercmd << "MATCH (n) WHERE n.Graphid=" << graphid << " OPTIONAL MATCH (n)-[r]-() DELETE n,r;";
		neo4cypher(cyphercmd.str().c_str());
		std::vector<struct anode>::iterator it;
		char buf[256];
		for (it = conceptnodes.begin(); it != conceptnodes.end(); ++it){
			sprintf_s(buf, "c%u", (*it).nodeid);
			AddNodetoNeo4j((*it).word.c_str(), 1, buf, &(*it).extra, graphid, 0);
		}
		for (it = relationnodes.begin(); it != relationnodes.end(); ++it){
			sprintf_s(buf, "r%u", (*it).nodeid);
			AddNodetoNeo4j((*it).word.c_str(), 2, buf, &(*it).extra, graphid, (*it).sentid);
		}
		for (std::vector<struct link>::iterator lit = links.begin(); lit != links.end(); ++lit){
			int sbj	,  obj , vb;
			sbj = obj = vb = -1;
			for (unsigned int i = 0; i < conceptnodes.size(); ++i){
				if (conceptnodes[i].nodeid == (*lit).sbj)
					sbj = i;
				else if (conceptnodes[i].nodeid == (*lit).obj)
					obj = i;
				if (sbj != -1 && obj != -1)
					break;
			}
			for (unsigned int i = 0; i < relationnodes.size(); ++i){
				if (relationnodes[i].nodeid == (*lit).vb){
					vb = i;
					break;
				}
			}
			MakeLink(1, conceptnodes[sbj].extra, relationnodes[vb].extra);
			MakeLink(2, relationnodes[vb].extra, conceptnodes[obj].extra);
		}
		printf_s("all done\n");
	}
	return true;

}

bool Neo4jInteract::neo4cyphermultiple(std::vector<std::string> *commands, std::string *response){
	//Sends a list of commands to the Neo4j database in one go
	*response = "";
	if (!curlok)
		return false;
	CURL *curl;
	curl = curl_easy_init();
	if (!curl)
		return false;
	CURLcode res;

	rapidjson::Document d;
	d.SetObject();
	rapidjson::Document statements;
	statements.SetArray();

	std::vector<rapidjson::Document> astate;
	astate.resize(commands->size());
	for (unsigned int i = 0; i < astate.size(); ++i){
		astate[i].SetObject();
		rapidjson::Value valObjectString(rapidjson::kStringType);
		valObjectString.SetString((commands->begin()+i)->c_str(), astate[i].GetAllocator());
		astate[i].AddMember("statement", valObjectString, astate[i].GetAllocator());
		statements.PushBack(astate[i], statements.GetAllocator());
	}
	d.AddMember("statements", statements, d.GetAllocator());
	rapidjson::StringBuffer strbuf;
	rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
	d.Accept(writer);
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, "charsets: utf-8");
	headers = curl_slist_append(headers, authenttoken.c_str());
	std::stringstream labelupdate;
	labelupdate << neo4jlocation << "db/data/transaction/commit";
	curl_easy_setopt(curl, CURLOPT_URL, labelupdate.str().c_str());
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallbackString);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)response);

	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strbuf.GetString());
	res = curl_easy_perform(curl);
	long http_code = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
	if (res != CURLE_OK){
		printf_s("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		curl_easy_cleanup(curl);
		return false;
	}
	curl_easy_cleanup(curl);
	return true;
}

bool Neo4jInteract::neo4cypher(const char *command, std::string *response){
	//To send a single query to the neo4j database
	if (!curlok)
		return false;
	CURL *curl;
	CURLcode res;
	std::stringstream json;
	json << "{ \"statements\" :[{ \"statement\" : \"" << command << "\"}]	}";
	//struct MemoryStruct chunk;

	rapidjson::Document d;
	d.Parse(json.str().c_str());
	rapidjson::StringBuffer strbuf;
	rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
	d.Accept(writer);

	curl = curl_easy_init();
	if (!curl)
		return false;
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, "charsets: utf-8");
	headers = curl_slist_append(headers, authenttoken.c_str());
	std::stringstream labelupdate;
	labelupdate << neo4jlocation << "db/data/transaction/commit";
	curl_easy_setopt(curl, CURLOPT_URL, labelupdate.str().c_str());
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallbackString);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)response);

	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strbuf.GetString());
	res = curl_easy_perform(curl);
	long http_code = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
	if (res != CURLE_OK){
		printf_s("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		curl_easy_cleanup(curl);
		return false;
	}
	curl_easy_cleanup(curl);
	return true;
}

bool Neo4jInteract::neo4cypher(const char *command){
	if (!curlok)
		return false;
	CURL *curl;
	CURLcode res;
	std::stringstream json;
	json << "{ \"statements\" :[{ \"statement\" : \"" << command << "\"}]	}";
	struct MemoryStruct chunk;

	rapidjson::Document d;
	d.Parse(json.str().c_str());
	rapidjson::StringBuffer strbuf;
	rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
	d.Accept(writer);

	curl = curl_easy_init();
	if (!curl)
		return false;
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, "charsets: utf-8");
	headers = curl_slist_append(headers, authenttoken.c_str());
	std::stringstream labelupdate;
	labelupdate << neo4jlocation << "db/data/transaction/commit";
	curl_easy_setopt(curl, CURLOPT_URL, labelupdate.str().c_str());
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strbuf.GetString());
	res = curl_easy_perform(curl);
	long http_code = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
	if (res != CURLE_OK){
		printf_s("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		curl_easy_cleanup(curl);
		free(chunk.memory);
		return false;
	}
	curl_easy_cleanup(curl);
	free(chunk.memory);
	return true;
}

bool Neo4jInteract::AddLinktoList(int type, long nodeid, long nodeto){
	//Adds a link to the temporary list to prepare the REST api call to Neo4j
	linklist.emplace_back();
	linklistnest.emplace_back();
	int which = linklist.size()-1;
	linklist[which].SetObject();
	linklistnest[which].SetObject();
	rapidjson::Document::AllocatorType&allocator = linklist[which].GetAllocator();
	rapidjson::Value valObjectString(rapidjson::kStringType);
	rapidjson::Document::AllocatorType&nestallocator = linklistnest[which].GetAllocator();
	char buf[128];
	linklist[which].AddMember("method","POST", allocator);
	sprintf_s(buf, "/node/%d/relationships", nodeid);
	valObjectString.SetString(buf, allocator);
	linklist[which].AddMember("to", valObjectString, allocator);
	sprintf_s(buf, "/node/%d", nodeto);
	valObjectString.SetString(buf, nestallocator);
	linklistnest[which].AddMember("to", valObjectString, nestallocator);
	if (type == 1)
		linklistnest[which].AddMember("type", "sbj", nestallocator);
	else
		linklistnest[which].AddMember("type", "obj", nestallocator);
	linklist[which].AddMember("body", linklistnest[which], allocator);
	rapidjson::Document::AllocatorType&fullallocator = fulllinks->GetAllocator();
	fulllinks->PushBack(linklist[which], fullallocator);
	return true;
}

bool Neo4jInteract::MakeLink(int type, long nodeid, long nodeto){
	/*
	Inserts a single link into a Neo4j database. Type (subject or object), nodefrom and nodeto are needed
	*/
	if (!curlok)
		return false;
	CURL *curl;
	CURLcode res;
	rapidjson::Document d;
	d.SetObject();
	rapidjson::Document::AllocatorType&allocator = d.GetAllocator();
	rapidjson::Value valObjectString(rapidjson::kStringType);
	std::stringstream url;
	url << neo4jlocation << "db/data/node/" << nodeto;
	valObjectString.SetString(url.str().c_str(), allocator);
	d.AddMember("to", valObjectString, allocator);
	if (type == 1)
		d.AddMember("type", "sbj", allocator);
	else
		d.AddMember("type", "obj", allocator);
	rapidjson::StringBuffer strbuf;
	rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
	d.Accept(writer);

	struct MemoryStruct chunk;

	curl = curl_easy_init();
	if (!curl)
		return false;
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, "charsets: utf-8");
	headers = curl_slist_append(headers, authenttoken.c_str());
	std::stringstream labelupdate;
	labelupdate << neo4jlocation << "db/data/node/" << nodeid << "/relationships";
	curl_easy_setopt(curl, CURLOPT_URL, labelupdate.str().c_str());
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strbuf.GetString());
	res = curl_easy_perform(curl);
	long http_code = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
	if (res != CURLE_OK){
		printf_s("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		curl_easy_cleanup(curl);
		free(chunk.memory);
		return false;
	}
	curl_easy_cleanup(curl);
	free(chunk.memory);
	return true;
}

bool Neo4jInteract::isopen(void){
	return curlok;
}