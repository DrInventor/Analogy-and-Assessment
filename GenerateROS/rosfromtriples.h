#include <string>
#include <vector>

#include "neo4jinteract.h"
#include "drinventorsql.h"
#include "commonstructs.h"

#if !(defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__))
#define stricmp strcasecmp
#define printf_s printf
#define fprintf_s fprintf
#define sprintf_s sprintf
#define sscanf_s sscanf
#endif

class ROSFromTriples{
public:
	//ROSFromTriples(const std::vector<std::string>, const std::vector<std::string>, const std::vector<std::string>, const std::vector<long>, long);
	ROSFromTriples(std::vector<struct triplesent> *, long);
	~ROSFromTriples();
	void PrintGVFile(const char *filename, bool makepng, bool Neo4j, const char *dirtxt);
	void InserttoNeo4j(void);
	DrInventorSqlitedb *sqlitedb;
	Neo4jInteract *neodb;
	std::string gvlocation, neo4jlocation;
	std::vector<struct anode> concepts;
	std::vector<struct anode> relations;
	std::vector<struct link> links;
	bool IsCreated(void);
private:
	long graphid;
	/*struct triple{
		std::string vb, sbj, obj;
		long sentid;
	};*/
	std::vector<struct triplesent> *triples;
	bool linksmade;
	//std::vector<struct triple> triples;

	int find_concept(const char *);
	void makelinks(void);
	bool makeneo4jrelations(unsigned int);
	bool ok;

};