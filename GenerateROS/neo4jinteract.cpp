#include "neo4jinteract.h"

bool Neo4jInteract::AddNodetoNeo4j(const char *word, int type, const char *gvlab, long *nodeid, long graphid, long sentid){
	/*
	Adds a node to the Neo4j database. Uses rapidjson to form the json expression (could easily form without rapidjson but used anyway)
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

Neo4jInteract::Neo4jInteract(const char *location){
	/*
	Constructor
	Tests if Curl can make any connection with the database
	If it can't, all other database operations are done
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
		curl_easy_cleanup(curl);
		free(chunk.memory);
	}
}

void replaceinstring(std::string &toedit, const char *rpl, const char *with){
	size_t found;
	while ((found = toedit.find(rpl)) != std::string::npos){
		toedit.replace(found, strlen(rpl), with);
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
		replaceinstring(temp, "\t", "");
		replaceinstring(temp, " ", "");
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
//	printf_s(chunk.memory);
	free(chunk.memory);
	return true;
}

bool Neo4jInteract::MakeLink(int type, long nodeid, long nodeto){
	/*
	Insert a link into a Neo4j database. Type (subject or object), nodefrom and nodeto are needed
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