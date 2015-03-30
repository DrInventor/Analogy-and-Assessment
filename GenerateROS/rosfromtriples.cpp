#include "rosfromtriples.h"

#if !(defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__))
#define _stricmp strcasecmp
#endif

ROSFromTriples::~ROSFromTriples(){

}

int ROSFromTriples::find_concept(const char *tofind){
	/*
	Simple function to search if a concept already exists in memory and returns its position in the array
	STL and find could be used but easier to implement it this way since we want a case insensitive match
	*/
	for (unsigned int i = 0; i < concepts.size(); ++i){
		if (_stricmp(tofind, concepts[i].word.c_str()) == 0)
			return i;
	}
	return -1;
}

/*ROSFromTriples::ROSFromTriples(const std::vector<std::string> vb, const std::vector<std::string> sbj, const std::vector<std::string> obj, const std::vector<long> sentid, long gid){
	//Constructor: Takes in graphid (from SQL database) and copies the vb, sbj and obj arrays into a struct local to the class
	ok = false;
	if (vb.size() != sbj.size() || vb.size() != obj.size()){
		printf_s("Triples passed incorrectly\n");
	}
	else {
		triples.resize(vb.size());
		linksmade = false;
		graphid = gid;
		for (unsigned int i = 0; i < vb.size(); ++i){
			triples[i].vb = vb[i];
			triples[i].sbj = sbj[i];
			triples[i].obj = obj[i];
			triples[i].sentid = sentid[i];
		}
		ok = true; 
	}
}*/

ROSFromTriples::ROSFromTriples(std::vector<struct triplesent> *ptr, long gid){
	ok = false;
	linksmade = false;
	if (ptr != NULL){
		triples = ptr;
		graphid = gid;
		ok = true;
	}

}

void ROSFromTriples::makelinks(void){
	/*
	The main function of ROSFrom Triples, this creates the arrays of concepts and relationships and then the links array
	It ensures concept nodes are not repeated an only used once
	If there are unknown subjects or objects these are taken into account
	*/
	struct anode temp;
	unsigned int unknown = 0;
	for (std::vector<struct triplesent>::iterator it = triples->begin(); it != triples->end(); ++it){
		unsigned int rnumb = relations.size();
		temp.word = (*it).vb;
		temp.sentid = (*it).sentid;
		relations.emplace_back(temp);
		unsigned int snumb;
		int loc;
		if ((*it).sbj == "%%-1%%"){
			char buf[16];
			sprintf_s(buf, "[unknown]-%d", unknown);
			snumb = concepts.size();
			temp.word = buf;
			concepts.emplace_back(temp);
			++unknown;
		}
		else{
			if ((loc = find_concept((*it).sbj.c_str())) == -1){
				snumb = concepts.size();
				temp.word = (*it).sbj;
				concepts.emplace_back(temp);
			}
			else
				snumb = (unsigned int)loc;
		}
		unsigned int onumb;
		if ((*it).obj == "%%-1%%"){
			char buf[16];
			sprintf_s(buf, "[unknown]-%d", unknown);
			onumb = concepts.size();
			temp.word = buf;
			concepts.emplace_back(temp);
			++unknown;
		}
		else{
			if ((loc = find_concept((*it).obj.c_str())) == -1){
				onumb = concepts.size();
				temp.word = (*it).obj;
				concepts.emplace_back(temp);
			}
			else
				onumb = (unsigned int)loc;
		}
		links.emplace_back();
		std::vector<struct link>::iterator whichlink = links.end() - 1;
		(*whichlink).vb = rnumb;
		(*whichlink).sbj = snumb;
		(*whichlink).obj = onumb;
	}
	linksmade = true;
}

/*bool ROSFromTriples::makeneo4jrelations(unsigned int tlink){
	//A Relation has two links: one from subject to verb and then one from verb to object. Insert both these into neo4j database
	neodb->MakeLink(1, concepts[links[tlink].sbj].nodeid, relations[links[tlink].vb].nodeid);
	neodb->MakeLink(2, relations[links[tlink].vb].nodeid, concepts[links[tlink].obj].nodeid);
	return true;
}*/

bool ROSFromTriples::makeneo4jrelations(unsigned int tlink){
	//A Relation has two links: one from subject to verb and then one from verb to object. Insert both these into neo4j database
	neodb->AddLinktoList(1, concepts[links[tlink].sbj].nodeid, relations[links[tlink].vb].nodeid);
	neodb->AddLinktoList(2, relations[links[tlink].vb].nodeid, concepts[links[tlink].obj].nodeid);
	return true;
}

void ROSFromTriples::InserttoNeo4j(void){
	if (!linksmade)
		makelinks();
	neodb->BeginBatchOperations(concepts.size() + relations.size());
	char buf[64];
	for (unsigned int i = 0; i < concepts.size(); ++i){
		sprintf_s(buf, "c%u", i);
		neodb->AddNodetoList(i, concepts[i].word.c_str(), 1, buf, graphid, 0);
	}
	for (unsigned int i = 0; i < links.size(); ++i){
		sprintf_s(buf, "r%u", i);
		neodb->AddNodetoList(concepts.size() + i, relations[links[i].vb].word.c_str(), 2, buf, graphid, relations[links[i].vb].sentid);
	}
	std::vector<long> ids = neodb->ExecuteNodeList();
	for (unsigned int i = 0; i < ids.size(); ++i){
		if (i < concepts.size())
			concepts[i].nodeid = ids[i];
		else
			relations[i - concepts.size()].nodeid = ids[i];
	}
	for (unsigned int i = 0; i < links.size(); ++i)
		makeneo4jrelations(i);
	neodb->ExecuteLinkList();
	sqlitedb->UpdateNodeCount(graphid, relations.size(), concepts.size());
}

void ROSFromTriples::PrintGVFile(const char *filename, bool makepng, bool Neo4j, const char *dirtxt){
	/*
	While called PrintGVFile, this also deals with the Neo4j insertions
	makelinks() has to be called first and this is checked
	Firstly the concept nodes are listed and written to the .gv file, similarly the nodes are inserted into the Neo4j database
	Then the concept nodes are listed and their relationsips are written to the .gv file and then inserted into Neo4j
	The png file is generated (using graphviz) if desired - this requires the graphviz location
	Finally, the sqlite database is updated with the number of nodes in the graph
	*/
	if (!linksmade)
		makelinks();
	FILE *out;
	std::ofstream cnodef, rnodef, linksf;
	out = fopen(filename, "w");
	std::stringstream filewrite;
	filewrite << dirtxt << "conceptlist";
	cnodef.open(filewrite.str().c_str());
	filewrite.str("");
	filewrite << dirtxt << "relationlist";
	rnodef.open(filewrite.str().c_str());
	filewrite.str("");
	filewrite << dirtxt << "linklist";
	linksf.open(filewrite.str().c_str());
	if (out){
		if(Neo4j) neodb->BeginBatchOperations(concepts.size() + relations.size());
		fprintf_s(out, "digraph graphname {\n\tnode [style=filled color=green]\n");
		for (unsigned int i = 0; i < concepts.size(); ++i){
			fprintf_s(out, "\tc%u [label=\"%s\" shape=box style=filled color=yellow]\n", i, concepts[i].word.c_str());
			char buf[256];
			sprintf_s(buf, "c%u", i);
			if (Neo4j) neodb->AddNodetoList(i, concepts[i].word.c_str(), 1, buf, graphid, 0);
			if (cnodef.is_open()){
				//If: so there is no blank newline at the end of the file
				if (i < concepts.size() - 1)
					cnodef << concepts[i].word << "\n";
				else
					cnodef << concepts[i].word;
			}
		}
		for (unsigned int i = 0; i < links.size(); ++i){
			fprintf_s(out, "\tr%u [label=\"%s\"];c%d -> r%d -> c%d;//SentID=%d\n", i, relations[links[i].vb].word.c_str(), links[i].sbj, i, links[i].obj, relations[links[i].vb].sentid);
			char buf[256];
			sprintf_s(buf, "r%u", i);
			if (Neo4j){
				neodb->AddNodetoList(concepts.size()+i,relations[links[i].vb].word.c_str(), 2, buf, graphid, relations[links[i].vb].sentid);
			}
			if (rnodef.is_open()){
				if (i < links.size() - 1)
					rnodef << relations[i].word << "\n";
				else
					rnodef << relations[i].word;
			}
			if (linksf.is_open()){
				if (i < links.size() - 1)
					linksf << links[i].vb << "," << links[i].sbj << "," << links[i].obj << "\n";
				else
					linksf << links[i].vb << "," << links[i].sbj << "," << links[i].obj;
			}
		}
		if (Neo4j){
			std::vector<long> ids = neodb->ExecuteNodeList();
			for (unsigned int i = 0; i < ids.size(); ++i){
				if (i < concepts.size())
					concepts[i].nodeid = ids[i];
				else
					relations[i - concepts.size()].nodeid = ids[i];
			}
			for (unsigned int i = 0; i < links.size(); ++i)
				makeneo4jrelations(i);
			neodb->ExecuteLinkList();
		}
		if (cnodef.is_open())
			cnodef.close();
		if (rnodef.is_open())
			rnodef.close();
		if (linksf.is_open())
			linksf.close();
		fprintf_s(out, "\tlabel=\"[*] indicates an unknown concept node\";\n\tlabelloc = bottom;\n\tlabeljust = right;\n}");
		fclose(out);
		if (makepng && gvlocation != ""){
			std::stringstream buf2;
			if(gvlocation != ".")
				buf2 << gvlocation << "dot " << filename << " -o " << filename << ".gif -T gif";
			else
				buf2 << "dot " << filename << " -o " << filename << ".gif -T gif";
			system(buf2.str().c_str());
		}
		sqlitedb->UpdateNodeCount(graphid, relations.size(), concepts.size());
	}
}

bool ROSFromTriples::IsCreated(void){
	return ok;
}
