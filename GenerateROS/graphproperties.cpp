#include "graphproperties.h"

/*
Graph Properties is for obtaining information from the Neo4j database
It is given an id (in either form of the Neo4j database or the longid (which is cross referenced in the sql database), the graph is then read and formed
An additional function ConvertGraphtoNodes can be called and then the local node array is populated with detailed information about each node
*/

GraphProperties::GraphProperties(long neo4id, const char *neo4loc, const char *neouser, const char *neopass, bool testneo4j){
	OK = false;
	GraphID = neo4id;
	neo4user = neouser; neo4pass = neopass;
	if (GetGraphbyID(neo4id, neo4loc, testneo4j) && conceptcount > 0)
			OK = true;
}

GraphProperties::GraphProperties(const char * longid, const char *dbfile, const char *neo4loc, const char *neouser, const char *neopass, bool testneo4j){
	DrInventorSqlitedb *sqlitedb;
	sqlitedb = new DrInventorSqlitedb(dbfile);
	long neo4id;
	GraphID = 0;
	OK = false;
	if (sqlitedb->isopen()){
		neo4id = sqlitedb->GetIDfromLongID(longid);
		delete sqlitedb;
		neo4user = neouser; neo4pass = neopass;
		if (GetGraphbyID(neo4id, neo4loc, testneo4j) && conceptcount > 0)
				OK = true;
	}
	else
		delete sqlitedb;
}

bool GraphProperties::is_open(void){
	return OK;
}

bool GraphProperties::GetGraphbyID(long id, const char *neo4loc, bool testneo4j){
	Neo4jInteract *neodb; 
	neodb = new Neo4jInteract(neo4loc, neo4user.c_str(), neo4pass.c_str(), testneo4j);
	if (neodb->isopen()){
		std::stringstream tmpcmd;
		std::vector<std::string> commands;
		tmpcmd << "match (n:Noun) where n.Graphid=" << id << " return count(n); ";
		commands.push_back(tmpcmd.str());
		tmpcmd.str("");
		tmpcmd << "match (n:Noun) where n.Graphid=" << id << " return n.Word, n.GVLabel, id(n);";
		commands.push_back(tmpcmd.str());
		tmpcmd.str("");
		tmpcmd << "match (n:Verb) where n.Graphid=" << id << " return count(n);";
		commands.push_back(tmpcmd.str());
		tmpcmd.str("");
		tmpcmd << "match (n:Verb) where n.Graphid=" << id << " return n.Word, n.GVLabel, n.SentID, id(n);";
		commands.push_back(tmpcmd.str());
		tmpcmd.str("");
		tmpcmd << "match (n:Noun)-[r:sbj]-(m:Verb)-[p:obj]-(o:Noun) where n.Graphid=" << id << " return n.GVLabel,m.GVLabel,o.GVLabel;";
		commands.push_back(tmpcmd.str());
		tmpcmd.str("");
		std::string response = "";
		neodb->neo4cyphermultiple(&commands, &response);
		delete neodb;
		//Parsing Fun
		rapidjson::Document ret;
		ret.Parse(response.c_str());
		int noconcepts = 0, norelations = 0;
		std::vector<int> sentid;
		std::vector<struct triplesent> links;
		if (ret.IsObject() && ret.HasMember("results") && ret["results"].IsArray() && ret["results"].Size() == 5){
			//First get the counts, i.e. results[0] and results[2]
			for (unsigned int i = 0; i < 3; i += 2){
				if (ret["results"][i]["data"].IsArray() && ret["results"][i]["data"][0].HasMember("row")){
					rapidjson::StringBuffer sb;
					rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
					ret["results"][i]["data"].Accept(writer);
					std::string parsethis = sb.GetString();
					std::string tofind = "[{\"row\":[";
					size_t found = parsethis.find(tofind);
					if (found != std::string::npos){
						if (i == 0)
							noconcepts = atoi(parsethis.substr(found + tofind.size()).c_str());
						else
							norelations = atoi(parsethis.substr(found + tofind.size()).c_str());
					}
				}
			}
			//Now get the name of each Node. This assumes all concepts/relation noades are c0->cN and r0->rN respectively with no gaps
			bool needconlookup = false, needrellookup = false;
			thegraph.concepts.resize(noconcepts); thegraph.relations.resize(norelations); sentid.resize(norelations);
			std::vector<int> lookupc, lookupr;
			for (unsigned int i = 1; i < 4; i += 2){
				if (ret["results"][i]["data"].IsArray()){
					for (unsigned int k = 0; k < ret["results"][i]["data"].Size(); ++k){
						if (ret["results"][i]["data"][k].HasMember("row")){
							rapidjson::StringBuffer sb;
							rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
							ret["results"][i]["data"][k].Accept(writer);
							std::string parsethis = sb.GetString();
							std::string tofind = "{\"row\":[";
							size_t found = parsethis.find(tofind);
							if (found != std::string::npos){
								parsethis = parsethis.substr(found + tofind.size());
								parsethis = parsethis.substr(0, parsethis.find("]}"));
							}
							std::vector<std::string> parsed = splitbydelimiter(parsethis, ",", true);
							int temp;
							if (i == 1){
								sscanf_s(parsed[1].c_str(), "c%d", &temp);
								if (temp >= noconcepts){//i.e. is the c0-cN sequence is broken? 
									needconlookup = true;
									break; //Since the sequence is broken, exit the loop as a new method is required
								}
								else{
									thegraph.concepts[temp].word = parsed[0];
									thegraph.concepts[temp].id = atoi(parsed[2].c_str());
								}
							}
							else{
								sscanf_s(parsed[1].c_str(), "r%d", &temp);
								if (temp >= norelations){ 
									needrellookup = true;
									break;
								}
								else{
									thegraph.relations[temp].word = parsed[0];
									sentid[temp] = atoi(parsed[2].c_str());
									thegraph.relations[temp].id = atoi(parsed[3].c_str());
								}
							}
						}
					}
				}
			}
			//This is only called if the c0->cN sequence is broken (so gaps are actually allowed)
			if (needconlookup){
				lookupc.resize(noconcepts);
				int i = 1;
				if (ret["results"][i]["data"].IsArray()){
					for (unsigned int k = 0; k < ret["results"][i]["data"].Size(); ++k){
						if (ret["results"][i]["data"][k].HasMember("row")){
							rapidjson::StringBuffer sb;
							rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
							ret["results"][i]["data"][k].Accept(writer);
							std::string parsethis = sb.GetString();
							std::string tofind = "{\"row\":[";
							size_t found = parsethis.find(tofind);
							if (found != std::string::npos){
								parsethis = parsethis.substr(found + tofind.size());
								parsethis = parsethis.substr(0, parsethis.find("]}"));
							}
							std::vector<std::string> parsed = splitbydelimiter(parsethis, ",", true);
							int temp;
							sscanf_s(parsed[1].c_str(), "c%d", &temp);
							lookupc[k] = temp;
							thegraph.concepts[k].word = parsed[0];
							thegraph.concepts[k].id = atoi(parsed[2].c_str());
						}
					}
				}
			}
			if (needrellookup){
				lookupc.resize(noconcepts);
				int i = 3;
				if (ret["results"][i]["data"].IsArray()){
					for (unsigned int k = 0; k < ret["results"][i]["data"].Size(); ++k){
						if (ret["results"][i]["data"][k].HasMember("row")){
							rapidjson::StringBuffer sb;
							rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
							ret["results"][i]["data"][k].Accept(writer);
							std::string parsethis = sb.GetString();
							std::string tofind = "{\"row\":[";
							size_t found = parsethis.find(tofind);
							if (found != std::string::npos){
								parsethis = parsethis.substr(found + tofind.size());
								parsethis = parsethis.substr(0, parsethis.find("]}"));
							}
							std::vector<std::string> parsed = splitbydelimiter(parsethis, ",", true);
							int temp;
							sscanf_s(parsed[1].c_str(), "c%d", &temp);
							lookupr[k] = temp;
							thegraph.relations[k].word = parsed[0];
							sentid[k] = atoi(parsed[2].c_str());
							thegraph.relations[k].id = atoi(parsed[3].c_str());
						}
					}
				}
			}
			struct link templink;
			//Finally, Parse the links Store the links
			if (ret["results"][4]["data"].IsArray()){
				for (unsigned int k = 0; k < ret["results"][4]["data"].Size(); ++k){
					if (ret["results"][4]["data"][k].HasMember("row")){
						rapidjson::StringBuffer sb;
						rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
						ret["results"][4]["data"][k].Accept(writer);
						std::string parsethis = sb.GetString();
						std::string tofind = "{\"row\":[";
						size_t found = parsethis.find(tofind);
						if (found != std::string::npos){
							parsethis = parsethis.substr(found + tofind.size());
							parsethis = parsethis.substr(0, parsethis.find("]}"));
						}
						std::vector<std::string> parsed = splitbydelimiter(parsethis, ",", true);
						int temp;
						if (needconlookup) temp = std::find(lookupc.begin(), lookupc.end(), atoi(parsed[0].substr(1).c_str())) - lookupc.begin();
						else temp = atoi(parsed[0].substr(1).c_str());
						templink.sbj = temp;
						if (needconlookup) temp = std::find(lookupc.begin(), lookupc.end(), atoi(parsed[2].substr(1).c_str())) - lookupc.begin();
						else temp = atoi(parsed[2].substr(1).c_str());
						templink.obj = temp;
						if (needrellookup) temp = std::find(lookupr.begin(), lookupr.end(), atoi(parsed[1].substr(1).c_str())) - lookupr.begin();
						else temp = atoi(parsed[1].substr(1).c_str());
						templink.vb = temp;
						templink.sentid = sentid[templink.vb];
						thegraph.links.push_back(templink);
					}
				}
			}
		}
		conceptcount = (long)thegraph.concepts.size();
		return true;
	}
	else
		delete neodb;
	conceptcount = (long)thegraph.concepts.size();
	return false;
}

void GraphProperties::ConvertGraphtoNodes(void){
	nodes.resize(thegraph.concepts.size() + thegraph.relations.size());
	for (std::vector<struct link>::iterator it = thegraph.links.begin(); it != thegraph.links.end(); ++it){
		++nodes[(*it).sbj].outDegree;
		++nodes[(*it).obj].inDegree;
		nodes[(*it).sbj].outNodeIds.push_back((*it).vb + conceptcount);
		nodes[(*it).obj].inNodeIds.push_back((*it).vb + conceptcount);
		nodes[(*it).vb + conceptcount].inDegree = 1;
		nodes[(*it).vb + conceptcount].outDegree = 1;
		nodes[(*it).vb + conceptcount].sentenceId = (*it).sentid;
		nodes[(*it).vb + conceptcount].outNodeIds.push_back((*it).obj);
		nodes[(*it).vb + conceptcount].inNodeIds.push_back((*it).sbj);
	}
	for (unsigned int i = 0; i < thegraph.concepts.size(); ++i){
		nodes[i].nodeId = thegraph.concepts[i].id;
		nodes[i].Word = thegraph.concepts[i].word;
		nodes[i].type = 0;
	}
	for (unsigned int i = 0; i < thegraph.relations.size(); ++i){
		nodes[i+conceptcount].nodeId = thegraph.relations[i].id;
		nodes[i+conceptcount].Word = thegraph.relations[i].word;
		nodes[i+conceptcount].type = 1;
	}
	for (std::vector<struct longnode>::iterator it = nodes.begin(); it != nodes.end(); ++it){
		(*it).degree = (*it).outDegree + (*it).inDegree;
		(*it).GraphId = GraphID;
	}
	/*for (std::vector<struct longnode>::iterator it = nodes.begin(); it != nodes.end(); ++it){
		printf_s("\n\n---------------------------------\nNodeid : %ld\nGraphID : %ld\nSentID : %ld\nType : %d\nWord : %s\nDegree : %d\n", (*it).nodeId, (*it).GraphId, (*it).sentenceId, (*it).type, (*it).Word.c_str(), (*it).degree);
		printf_s("InNodeIDs : ");
		for (unsigned int i = 0; i < (*it).inNodeIds.size(); ++i)
			printf_s("%ld ", (*it).inNodeIds[i]);
		printf_s("\nOutNodeIds : ");
		for (unsigned int i = 0; i < (*it).outNodeIds.size(); ++i)
			printf_s("%ld ", (*it).outNodeIds[i]);
	}*/
}


GraphProperties::~GraphProperties(void){
}