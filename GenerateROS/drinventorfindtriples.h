#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

#include "commonstructs.h"

#if !(defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__))
#define stricmp strcasecmp
#define printf_s printf
#define fprintf_s fprintf
#define sprintf_s sprintf
#define sscanf_s sscanf
#endif

class DrInventorFindTriples{
public:
	DrInventorFindTriples(const char *, const char *);
	DrInventorFindTriples(const char *);
	~DrInventorFindTriples();
	bool PrintInbetweenGraphs(void);
	void MakeNewLinks(bool tc=false);
	void WriteTriplesToCSVFile(const char *file);
	//void GiveTriples(std::vector<std::string> *, std::vector<std::string> *, std::vector<std::string> *, std::vector<long> *);
	void MakeTripleSent(void);
	bool isopen(void);
	std::vector<struct triplesent> triplesent;
private:
	bool fisopen;
	struct link{
		int from, to;
		std::string relat;
		bool checked;
	};

	struct word{
		unsigned int id;
		std::string st,pos;
		int type; //0=discard, 1=verb, 2=noun
	};

	struct stringpair{
		std::string st1, st2;
	};

	struct triple{
		int sbj, obj, ver;
	};

	struct sentence{
		long id;
		std::vector<struct link> links;
		std::vector<struct word> words;
		std::vector<struct link> newlinks;
		std::vector<struct triple> triples;
	};
	bool tagwords(void);
	bool tagwordsnew(void);
	std::vector<struct sentence> sentences;
	std::vector<struct stringpair> appos;
	std::string filename, tokenfile;
	bool graphsmade, foundtriples, blockprint, tagwordsold;
	
	bool findappo(std::vector<struct sentence>::iterator sit, unsigned int id);
	void FindTriples(bool tc=false);
	int processline(std::string);
	int processlineofcombinedtable(std::string which);
	std::vector<int> containedin(int bit, int sent);
	void discardstuff(int sent);
	void findnewlink(unsigned int start, unsigned int from, std::vector<struct link> *newlinks, unsigned int sent);
	void combineVC(void);
	struct find_sID
	{
		int id;
		find_sID(int id) : id(id) {}
		bool operator () (const sentence& m) const
		{
			return m.id == id;
		}
	};

	struct find_Word{
		unsigned int id;
		find_Word(unsigned int id) : id(id) {}
		bool operator () (const word& m) const
		{
			return m.id == id;
		}
	};

	int find_word(int id, std::vector<struct word> &which){
		for (unsigned int i = 0; i < which.size(); ++i)
		if (id == which[i].id) return i;
		return -1;
	}
};