#include "graphinterface.h"

GraphInterface::GraphInterface(std::vector<struct anode> *concepts, std::vector<struct anode> *relations, std::vector<struct link> *links, long gid){
	graphid = gid;
	vertex_conc.resize(concepts->size());
	//Add all concepts and relation nodes to the BoostGraph
	for (unsigned int i = 0; i < concepts->size(); ++i)
		vertex_conc[i] = add_vertex(g);
	vertex_relat.resize(relations->size());
	for (unsigned int i = 0; i < relations->size(); ++i)
		vertex_relat[i] = add_vertex(g);
	//Add the edges to the boost graph
	for (std::vector<struct link>::iterator it = links->begin(); it != links->end(); ++it){
		add_edge(vertex_conc[(*it).sbj], vertex_relat[(*it).vb], g);
		add_edge(vertex_relat[(*it).vb], vertex_conc[(*it).obj], g);
	}
	//Gets the properties and stores them in the object
	std::stringstream tooutput;
	std::vector<int> component(num_vertices(g));
	int num = connected_components(g, &component[0]);
	HighestDegree();
	for (unsigned int i = 0; i < highestdegree.size(); ++i){
		highestdegreestring.emplace_back(concepts->at(highestdegree[i]).word);
	}
	LargestComponent(&component, num);
	nouniquerelat = FindUniqueRelations(relations, &mostcommonverb);
	for (unsigned int i = 0; i < mostcommonverb.size(); ++i)
		printf("Most Common Verb: %s\n", mostcommonverb[i].c_str());
	nocomponents = num;
}

int GraphInterface::FindUniqueRelations(std::vector<struct anode> *relations, std::vector<std::string> *mostcommon){
	std::vector<std::string> uniques;
	std::vector<int> count;
	for (unsigned int i = 0; i < relations->size(); ++i){
		std::vector<std::string>::iterator it = std::find(uniques.begin(), uniques.end(), relations->at(i).word);
		if (it == uniques.end()){
			uniques.emplace_back(relations->at(i).word);
			count.emplace_back(1);
		}
		else{
			++count[it - uniques.begin()];
		}
	}
	int currhigh = 0;
	std::vector<int> toreturn;
	for (unsigned int i = 0; i < count.size(); ++i){
		if (count[i] > currhigh){
			if (toreturn.size() > 1)
				std::vector<int>().swap(toreturn);
			if (toreturn.size() == 0)
				toreturn.emplace_back(i);
			else
				toreturn[0] = i;
			currhigh = count[i];
		}
		else if (count[i] == currhigh){
			toreturn.emplace_back(i);
		}
	}
	for (unsigned int i = 0; i < toreturn.size(); ++i){
		mostcommon->emplace_back(uniques[toreturn[i]]);
	}
	return uniques.size();
}

void GraphInterface::LargestComponent(std::vector<int> *comp, int num){
	int *compsize = new int[num]();
	//Find the size of each component
	for (unsigned int i = 0; i < comp->size(); ++i){
		++compsize[comp->at(i)];
	}
	int currhigh = 0;
	for (int i = 0; i < num; ++i){
		if (compsize[i] > currhigh){
			if (largestcomponents.size() > 1)
				std::vector<int>().swap(largestcomponents);
			if (largestcomponents.size() == 0)
				largestcomponents.emplace_back(i);
			else
				largestcomponents[0] = i;
			currhigh = compsize[i];
		}
		else if (compsize[i] == currhigh){
			largestcomponents.emplace_back(i);
		}
	}
	lrgcomp = currhigh;
	delete[] compsize;
}

void GraphInterface::HighestDegree(void){
	/*
	From a Boost Graph, it returns the node(s) that have the highest degree (indegree+outdegree)
	Only Concept nodes are checked
	*/
	int currhigh = 0;
	int is;
	for (unsigned int i = 0; i < vertex_conc.size(); ++i){
		is = boost::degree(vertex_conc[i], g);
		if (is > currhigh){
			if (highestdegree.size() > 1)
				std::vector<int>().swap(highestdegree);
			if (highestdegree.size() == 0)
				highestdegree.emplace_back(i);
			else
				highestdegree[0] = i;
			currhigh = is;
		}
		else if (is == currhigh){
			highestdegree.emplace_back(i);
		}
	}
	hghdeg = currhigh;
}