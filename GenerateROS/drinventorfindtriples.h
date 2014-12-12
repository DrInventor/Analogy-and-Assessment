#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

class DrInventorFindTriples{
public:
	DrInventorFindTriples(const char *, const char *);
	~DrInventorFindTriples();
	bool PrintInbetweenGraphs(void);
	void MakeNewLinks(void);
	void WriteTriplesToCSVFile(const char *file);
	void GiveTriples(std::vector<std::string> *, std::vector<std::string> *, std::vector<std::string> *);
	bool isopen(void);
private:
	bool fisopen;
	struct link{
		int from, to;
		std::string relat;
		bool checked;
	};

	struct word{
		unsigned int id;
		std::string st;
		int type; //0=discard, 1=verb, 2=noun
	};

	struct triple{
		int sbj, obj, ver;
	};

	struct sentence{
		int id;
		std::vector<struct link> links;
		std::vector<struct word> words;
		std::vector<struct link> newlinks;
		std::vector<struct triple> triples;
	};
	bool tagwords(void);
	std::vector<struct sentence> sentences;
	std::string filename, tokenfile;
	bool graphsmade, foundtriples, blockprint;

	void FindTriples(void);
	int processline(std::string);
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