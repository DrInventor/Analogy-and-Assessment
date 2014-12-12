#include <string>
#include <vector>

#include "neo4jinteract.h"
#include "drinventorsql.h"
#include "commonstructs.h"

class ROSFromTriples{
public:
	ROSFromTriples(const std::vector<std::string>, const std::vector<std::string>, const std::vector<std::string>, long);
	~ROSFromTriples();
	void PrintGVFile(const char *filename, bool makepng, bool Neo4j, const char *dirtxt);
	DrInventorSqlitedb *sqlitedb;
	Neo4jInteract *neodb;
	std::string gvlocation, neo4jlocation;
	std::vector<struct anode> concepts;
	std::vector<struct anode> relations;
	std::vector<struct link> links;
private:
	long graphid;
	struct triple{
		std::string vb, sbj, obj;
	};
	bool linksmade;
	std::vector<struct triple> triples;

	int find_concept(const char *);
	void makelinks(void);
	bool makeneo4jrelations(unsigned int);

};