#include <vector>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>

#include "commonstructs.h"

class GraphInterface{
public:
	GraphInterface(std::vector<struct anode> *, std::vector<struct anode> *, std::vector<struct link> *, long gid);
	std::vector<int> largestcomponents, highestdegree;
	std::vector<std::string> highestdegreestring, mostcommonverb;
	int lrgcomp, hghdeg, nouniquerelat, nocomponents;
private:
	long graphid;
	typedef boost::adjacency_list <boost::vecS, boost::vecS, boost::undirectedS> Graph;
	Graph g;
	std::vector<boost::graph_traits<Graph>::vertex_descriptor> vertex_conc, vertex_relat;
	void HighestDegree(void);
	void LargestComponent(std::vector<int> *comp, int num);
	int FindUniqueRelations(std::vector<struct anode> *, std::vector<std::string> *);

};