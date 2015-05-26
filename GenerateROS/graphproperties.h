#include "commonstructs.h"
#include "neo4jinteract.h"
#include "drinventorsql.h"

class GraphProperties{
public:
	GraphProperties(long neo4id, const char *neoloc, const char *neouser, const char *neopass, bool testneo4j=true);
	GraphProperties(const char *longid, const char *dbfile, const char *neoloc, const char *neouser, const char *neopass, bool testneo4j=true);
	~GraphProperties(void);
	void ConvertGraphtoNodes(void);
	bool is_open(void);
	struct longnode{
		long nodeId, GraphId, sentenceId;
		int type;
		std::string Word;
		int	inDegree, outDegree, degree;
		std::vector<long> inNodeIds, outNodeIds;
		longnode(void){
			inDegree = outDegree = 0;
			sentenceId = -1;
		}
	};
	std::vector<struct longnode> nodes;
	struct agraph thegraph;
private:
	long GraphID;
	long conceptcount;
	bool OK;
	std::string neo4user, neo4pass;
	bool GetGraphbyID(long id, const char *neo4loc, bool testneo4j);
};