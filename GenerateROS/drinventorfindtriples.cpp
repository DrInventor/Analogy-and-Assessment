#include "drinventorfindtriples.h"

DrInventorFindTriples::~DrInventorFindTriples(){
	//printf("Destructor\n");
}

void DrInventorFindTriples::GiveTriples(std::vector<std::string> *vb, std::vector<std::string> *sbj, std::vector<std::string> *obj){
	/*
	Outputs the triples to separate vectors, should be passed pointers.
	The triples have to first be found, so if not done yet, do it now
	*/
	if (!foundtriples)
		FindTriples();
	for (std::vector<struct sentence>::iterator sit = sentences.begin(); sit != sentences.end(); ++sit){
		for (std::vector<struct triple>::iterator it = (*sit).triples.begin(); it != (*sit).triples.end(); ++it){
			vb->emplace_back((*sit).words[(*it).ver].st);
			if ((*it).sbj == -1)
				sbj->emplace_back("%%-1%%");
			else
				sbj->emplace_back((*sit).words[(*it).sbj].st);
			if ((*it).obj == -1)
				obj->emplace_back("%%-1%%");
			else
				obj->emplace_back((*sit).words[(*it).obj].st);
		}
	}
}

DrInventorFindTriples::DrInventorFindTriples(const char *file, const char *tknfile){
	/*
	Constructor, stores the main file and tknfile name and checks if they both exist (and are readable)
	If they exist, the main file is read in line by line and the first graphs (plain white) are made
	This is done line by line (line (titles) 0 ignored)
	*/
	filename = file;
	tokenfile = tknfile;
	std::ifstream toread;
	toread.open(file);
	graphsmade = false;
	blockprint = false;
	if (!toread.is_open()){
		fisopen = false;
	}
	else{
		std::ifstream toread2;
		toread2.open(tknfile);
		if (!toread2.is_open()){
			toread.close();
			fisopen = false;
		}
		else{
			toread2.close();
			fisopen = true;
			std::string temp;
			getline(toread, temp);
			while (getline(toread, temp))
				processline(temp);
		}
	}
}

bool DrInventorFindTriples::PrintInbetweenGraphs(void){
	/*
	This function generally should not be used, it is for debugging (and documenting) purposes
	It prints the "inbetween" graphs, i.e. the graphs with firstly the ones formed by processing the mainfile
	Then it prints the graphs made after discarding anything that's not a noun or verb
	After combining verbs the graphs from the initial file will be broken (blockprint check)
	If the Graphs with only nouns and verbs with the verbs combined is wanted this function can be used, however the initial (plain white) graphs will not be generated since broken
	*/
	if (graphsmade){
		printf("inbetween, %d\n", sentences.size());
		for (unsigned int i = 0; i < sentences.size(); ++i){
			char buf[1024];
			FILE *out;
			if (!blockprint){
				sprintf(buf, "%s-%d.gv", filename.c_str(), i);
				out = fopen(buf, "w");
				if (out){
					fprintf(out, "digraph graphname {\n");
					for (unsigned int j = 0; j < sentences[i].words.size(); ++j)
						fprintf(out, "\t\tw%d [label=\"%s\"]\n", j, sentences[i].words[j].st.c_str());
					for (unsigned int j = 0; j < sentences[i].links.size(); ++j)
						fprintf(out, "\t\tw%d -> w%d [label=\"%s\"]\n", sentences[i].links[j].from, sentences[i].links[j].to, sentences[i].links[j].relat.c_str());
					fprintf(out, "}");
					fclose(out);
					char buf3[1024];
					sprintf(buf3, "c:\\users\\user\\documents\\gv\\dot %s -o %s.png -T png", buf, buf);
					system(buf3);
				}
			}
			sprintf(buf, "%s-n%d.gv", filename.c_str(), i);
			printf("a:%s\n", buf);
			out = fopen(buf, "w");
			if (out){
				fprintf(out, "digraph graphname {\n");
				for (unsigned int j = 0; j < sentences[i].words.size(); ++j){
					if (sentences[i].words[j].type != 0){
						fprintf(out, "\t\tw%d [label=\"%s\" ", j, sentences[i].words[j].st.c_str());
						if (sentences[i].words[j].type == 1) fprintf(out, "style=filled color=green]\n");
						else if (sentences[i].words[j].type == 2) fprintf(out, "shape=box style=filled color=yellow]\n");
					}
				}
				for (unsigned int j = 0; j < sentences[i].newlinks.size(); ++j)
					fprintf(out, "\t\tw%d -> w%d [label=\"%s\"]\n", sentences[i].newlinks[j].from, sentences[i].newlinks[j].to, sentences[i].newlinks[j].relat.c_str());
				fprintf(out, "}");
				fclose(out);
				char buf2[1024];
				sprintf(buf2, "c:\\users\\user\\documents\\gv\\dot %s -o %s.png -T png", buf, buf);
				system(buf2);
			}
			fclose(out);
		}
		return true;
	}
	else{
		printf("Graphs not made");
		return false;
	}
}

void DrInventorFindTriples::FindTriples(void){
	/*
	Extract the triples: , ensure links are only processed once
	1) Find a link with a verb in it. If the link is of type SBJ make a triple from this by
	(i) searching for an adjacent corresponding OBJ
	(ii) using any adjacent noun
	(iii) seeing if any adjacent verbs are only in another link with another noun and then combining verbs
	2) If the link with a verb in it and the link is of type OBJ make a triple from this by
	(i) searching for an adjacent corresponding SBJ
	(ii) using any adjacent noun
	3) After all SBJ and OBJ links have been dealt with, find any verbs with two unprocessed links to nouns and make triples from these
	*/
	struct triple temptriple;
	printf("searching for triples\n");
	for (std::vector<struct sentence>::iterator sit = sentences.begin(); sit != sentences.end(); ++sit){
		std::vector<std::vector<struct link>::iterator> processed;
		for (std::vector<struct link>::iterator it = (*sit).newlinks.begin(); it != (*sit).newlinks.end(); ++it){
			if (std::find(processed.begin(), processed.end(), it) == processed.end()){
				if ((*sit).words[(*it).from].type == 1 && (*it).relat == "SBJ"){
					processed.emplace_back(it);
					temptriple.sbj = (*it).to;
					temptriple.ver = (*it).from;
					temptriple.obj = -1;
					//Got a subject so find object;//Should a nit not in processed check be done??
					for (std::vector<struct link>::iterator nit = (*sit).newlinks.begin(); nit != (*sit).newlinks.end(); ++nit){
						if (nit != it && std::find(processed.begin(), processed.end(), nit) == processed.end() && (*it).from == (*nit).from && (*sit).words[(*nit).to].type == 2 && (*nit).relat == "OBJ"){
							processed.emplace_back(nit);
							temptriple.obj = (*nit).to;
							break;
						}
					}
					//If no luck, try finding an adjacent noun anyway
					if (temptriple.obj == -1){
						for (std::vector<struct link>::iterator nit = (*sit).newlinks.begin(); nit != (*sit).newlinks.end(); ++nit){
							if (nit != it && std::find(processed.begin(), processed.end(), nit) == processed.end() && (*it).from == (*nit).from && (*sit).words[(*nit).to].type == 2){
								processed.emplace_back(nit);
								temptriple.obj = (*nit).to;
								break;
							}
						}
					}
					//Still if no luck, maybe join two verbs to find an object?
					if (temptriple.obj == -1){
						bool keepgoing = true;
						for (std::vector<struct link>::iterator nit = (*sit).newlinks.begin(); keepgoing && nit != (*sit).newlinks.end(); ++nit){
							if (nit != it && std::find(processed.begin(), processed.end(), nit) == processed.end() && (*it).from == (*nit).from && (*sit).words[(*nit).to].type == 1){
								for (std::vector<struct link>::iterator anotit = (*sit).newlinks.begin(); keepgoing &&anotit != (*sit).newlinks.end(); ++anotit){
									if (anotit != it && anotit != nit && std::find(processed.begin(), processed.end(), anotit) == processed.end()){
										if ((*anotit).from == (*nit).to && (*sit).words[(*anotit).to].type == 2){
											int count = 0;
											for (unsigned int k = 0; k < (*sit).newlinks.size(); ++k){
												if ((*sit).newlinks[k].to == (*nit).to || (*sit).newlinks[k].from == (*nit).to)
													++count;
											}
											if (count > 2)
												break;
											else{
												(*sit).words[(*it).from].st += "_";
												(*sit).words[(*it).from].st += (*sit).words[(*nit).to].st;
												for (std::vector<struct link>::iterator joinit = (*sit).newlinks.begin(); joinit != (*sit).newlinks.end(); ++joinit){
													if (joinit != nit){
														if ((*joinit).from == (*nit).to){
															(*joinit).from = (*nit).from;
															processed.emplace_back(joinit);
															temptriple.obj = (*joinit).to;
														}
													}
												}
												keepgoing = false;
											}
										}
									}
								}
							}
						}
					}
					(*sit).triples.emplace_back(temptriple);
				}
				else if ((*sit).words[(*it).from].type == 1 && (*it).relat == "OBJ"){
					processed.emplace_back(it);
					//Got an object
					temptriple.obj = (*it).to;
					temptriple.ver = (*it).from;
					temptriple.sbj = -1;
					//Got an object so find subject;
					for (std::vector<struct link>::iterator nit = (*sit).newlinks.begin(); nit != (*sit).newlinks.end(); ++nit){
						if (nit != it && std::find(processed.begin(), processed.end(), nit) == processed.end() && (*it).from == (*nit).from && (*sit).words[(*nit).to].type == 2 && (*nit).relat == "SBJ"){
							processed.emplace_back(nit);
							temptriple.sbj = (*nit).to;
							break;
						}
					}
					//If no luck, try finding an adjacent noun anyway
					if (temptriple.sbj == -1){
						for (std::vector<struct link>::iterator nit = (*sit).newlinks.begin(); nit != (*sit).newlinks.end(); ++nit){
							if (nit != it && std::find(processed.begin(), processed.end(), nit) == processed.end() && (*it).from == (*nit).from && (*sit).words[(*nit).to].type == 2){
								processed.emplace_back(nit);
								temptriple.sbj = (*nit).to;
								break;
							}
						}
					}
					(*sit).triples.emplace_back(temptriple);
				}
			}
		}
		//Having found dealt with all Sbj and Obj links. If any verbs have two adjacent nouns, add them as a triple
		for (std::vector<struct link>::iterator it = (*sit).newlinks.begin(); it != (*sit).newlinks.end(); ++it){
			if (std::find(processed.begin(), processed.end(), it) == processed.end()){
				if ((*sit).words[(*it).from].type == 1 && (*sit).words[(*it).to].type == 2){
					for (std::vector<struct link>::iterator nit = (*sit).newlinks.begin(); nit != (*sit).newlinks.end(); ++nit){
						if (nit != it && std::find(processed.begin(), processed.end(), nit) == processed.end()){
							if ((*it).from == (*nit).from && (*sit).words[(*nit).to].type == 2){
								processed.emplace_back(nit);
								temptriple.sbj = (*it).to;
								temptriple.ver = (*it).from;
								temptriple.obj = (*nit).to;
								(*sit).triples.emplace_back(temptriple);
							}
							else if ((*it).from == (*nit).to && (*sit).words[(*nit).from].type == 2){
								processed.emplace_back(nit);
								temptriple.sbj = (*it).to;
								temptriple.ver = (*it).from;
								temptriple.obj = (*nit).from;
								(*sit).triples.emplace_back(temptriple);
							}
						}
					}
				}
				else if ((*sit).words[(*it).to].type == 1 && (*sit).words[(*it).from].type == 2){
					for (std::vector<struct link>::iterator nit = (*sit).newlinks.begin(); nit != (*sit).newlinks.end(); ++nit){
						if (nit != it && std::find(processed.begin(), processed.end(), nit) == processed.end()){
							if ((*it).to == (*nit).from && (*sit).words[(*nit).to].type == 2){
								processed.emplace_back(nit);
								temptriple.sbj = (*it).from;
								temptriple.ver = (*it).to;
								temptriple.obj = (*nit).to;
								(*sit).triples.emplace_back(temptriple);
							}
							else if ((*it).to == (*nit).to && (*sit).words[(*nit).from].type == 2){
								processed.emplace_back(nit);
								temptriple.sbj = (*it).from;
								temptriple.ver = (*it).to;
								temptriple.obj = (*nit).to;
								(*sit).triples.emplace_back(temptriple);
							}
						}
					}
				}
			}
		}

	}
	foundtriples = true;
}

void DrInventorFindTriples::WriteTriplesToCSVFile(const char *file){
	//Not used, outputs the triples to a CSV file
	if (!foundtriples)
		FindTriples();
	FILE *out;
	out = fopen(file, "w");
	if (out){
		for (std::vector<struct sentence>::iterator sit = sentences.begin(); sit != sentences.end(); ++sit){
			for (std::vector<struct triple>::iterator it = (*sit).triples.begin(); it != (*sit).triples.end(); ++it){
				if ((*it).sbj == -1) fprintf(out, "\"%s\",\"%%%%-1%%%%\",\"%s\"\n", (*sit).words[(*it).ver].st.c_str(), (*sit).words[(*it).obj].st.c_str());
				else if ((*it).obj == -1) fprintf(out, "\"%s\",\"%s\",\"%%%%-1%%%%\"\n", (*sit).words[(*it).ver].st.c_str(), (*sit).words[(*it).sbj].st.c_str());
				else fprintf(out, "\"%s\",\"%s\",\"%s\"\n", (*sit).words[(*it).ver].st.c_str(), (*sit).words[(*it).sbj].st.c_str(), (*sit).words[(*it).obj].st.c_str());
			}
		}
		fclose(out);
	}
}

void DrInventorFindTriples::MakeNewLinks(void){
	/*
	Firstly all nodes that are not nouns or verbs are discarded
	Then some verb chains are combined
	*/
	if (tagwords()){
		for (unsigned int i = 0; i < sentences.size(); ++i)
			discardstuff(i);
		graphsmade = true;
		combineVC();
	}
	else
		printf("Tag Words failed");
}

void DrInventorFindTriples::combineVC(void){
	//Combine some verbs together, i.e. VC (Verb Chain), IM (infinitive), OPRD and NMOD
	for (std::vector<struct sentence>::iterator sit = sentences.begin(); sit != sentences.end(); ++sit){
		std::vector<std::vector<struct link>::iterator> todelete;
		for (std::vector<struct link>::iterator it = (*sit).newlinks.begin(); it != (*sit).newlinks.end(); ++it){
			if ((*it).relat == "VC" || (*it).relat == "IM" || ((*it).relat == "OPRD") || ((*it).relat == "NMOD")){
				if ((*sit).words[(*it).from].type == 1 && (*sit).words[(*it).to].type == 1){
					(*sit).words[(*it).from].st += "_";
					(*sit).words[(*it).from].st += (*sit).words[(*it).to].st;
					for (std::vector<struct link>::iterator nit = (*sit).newlinks.begin(); nit != (*sit).newlinks.end(); ++nit){
						if (nit != it){
							if ((*nit).from == (*it).to)
								(*nit).from = (*it).from;
						}
					}
					(*sit).words[(*it).to].type = 0;
					todelete.emplace_back(it);
				}
			}
		}
		if (todelete.size() > 0)
			blockprint = true;
		for (int i = todelete.size() - 1; i >= 0; --i){
			(*sit).newlinks.erase(todelete[i]);
		}
	}
}

void DrInventorFindTriples::findnewlink(unsigned int start, unsigned int from, std::vector<struct link> *newlinks, unsigned int sent){
	//function to determine "newlinks", these are the links made after non-nouns and non-verbs are discarded - ensures structure maintained
	std::vector<int> contained = containedin(from, sent);
	struct link templink;
	for (unsigned int i = 0; i < contained.size(); ++i){
		if (sentences[sent].words[sentences[sent].links[contained[i]].from].type != 0){
			templink.from = start;
			templink.to = sentences[sent].links[contained[i]].from;
			newlinks->emplace_back(templink);
		}
		else
			findnewlink(start, sentences[sent].links[contained[i]].from, newlinks, sent);
	}
}

void DrInventorFindTriples::discardstuff(int sent){
	//Get rid of nodes that are not nouns nor verbs, find new link is to make sure chains aren't broken
	struct link templink;
	for (unsigned int i = 0; i < sentences[sent].words.size(); ++i){
		if (sentences[sent].words[i].type != 0){
			std::vector<int> contained = containedin(i, sent);
			for (unsigned int j = 0; j < contained.size(); ++j){
				if (sentences[sent].words[sentences[sent].links[contained[j]].from].type != 0){
					templink.from = i;
					templink.to = sentences[sent].links[contained[j]].from;
					templink.relat = sentences[sent].links[contained[j]].relat;
					sentences[sent].newlinks.emplace_back(templink);
				}
				else
					findnewlink(i, sentences[sent].links[contained[j]].from, &sentences[sent].newlinks, sent);
			}
		}
	}
}

bool DrInventorFindTriples::tagwords(void){
	//Tag the nouns and verbs from the tknfile. If neither noun nor verb, that word will be discarded later
	std::ifstream toread;
	toread.open(tokenfile);
	if (!toread.is_open())
		return false;
	std::string leftover;
	getline(toread, leftover);
	while (getline(toread, leftover)){
		int token;
		std::string type;
		for (unsigned int i = 0; i < 4; ++i){
			std::string temp = leftover.substr(0, leftover.find('\t'));
			if (i != 3) leftover = leftover.substr(leftover.find('\t') + 1);
			//We only care about the first and last columns
			switch (i){
			case 0:
				sscanf(temp.c_str(), "\"%d\"", &token);
				break;
			case 1:
				break;
			case 2:
				break;
			case 3:
				type = temp.substr(1, temp.length() - 2);
				break;
			}
		}
		std::vector<struct word>::iterator wordit;
		bool keepgoing = true;
		for (unsigned int i = 0; keepgoing && i < sentences.size(); ++i){
			if ((wordit = std::find_if(sentences[i].words.begin(), sentences[i].words.end(), find_Word(token))) != sentences[i].words.end()){
				keepgoing = false;
				if (type.size() >= 2 && type.at(0) == 'N' && type.at(1) == 'N'){
					(*wordit).type = 2;
				}
				else if (type.size() >= 2 && type.at(0) == 'V' && type.at(1) == 'B')
					(*wordit).type = 1;
				else if (type.size() >= 3 && type.at(0) == 'P' && type.at(1) == 'R' && type.at(2) == 'P')
					(*wordit).type = 2;
				//New stuff
				else if (type.size() >= 2 && type.at(0) == 'E' && type.at(1) == 'X')
					(*wordit).type = 2;
				else if (type.size() >= 2 && type.at(0) == 'T' && type.at(1) == 'O'){
					//We sometimes want to keep the word "to", if for example it is part of an infinitive verb - it will be joined later in CombineVC
					(*wordit).type = 0;
					for (unsigned int j = 0; j < sentences[i].links.size(); ++j){
						if (wordit - sentences[i].words.begin() == sentences[i].links[j].to){
							if (sentences[i].links[j].relat == "IM"){
								(*wordit).type = 1;
								break;
							}
						}
					}
				}
				else
					(*wordit).type = 0;
			}
		}
	}
	toread.close();
	return true;
}

std::vector<int> DrInventorFindTriples::containedin(int bit, int sent){
	//Finds all the links a given node is in, this only goes in one direction since we are doing a tree search
	std::vector<int> toreturn;
	for (unsigned int i = 0; i < sentences[sent].links.size(); ++i){
		if (!sentences[sent].links[i].checked && sentences[sent].links[i].to == bit){
			toreturn.emplace_back(i);
			sentences[sent].links[i].checked = true;
		}
	}
	return toreturn;
}

int DrInventorFindTriples::processline(std::string which){
	/*
	Extract the words from the a line in the file
	Identifies the sentence id, the id of the word the link is from and the id of the word to. It also gets the actual words
	The sentence id is checked for uniqueness and if new a new sentence is created in the struct array
	Each word ID is then checked in sentences.words and again if new inserted
	The link from : "wordfrom to wordto" is then saved in terms of array positions
	*/
	int sID, FromID, ToID;
	std::string FromWord, ToWord, Relation, temp;
	std::string leftover = which;
	for (unsigned int i = 0; i < 8; ++i){
		temp = leftover.substr(0, leftover.find('\t'));
		if (i != 7) leftover = leftover.substr(leftover.find('\t') + 1);
		switch (i){
		case 0:
			sscanf(temp.c_str(), "\"%d\"", &sID);
			break;
		case 1:
			sscanf(temp.c_str(), "\"%d\"", &FromID);
			break;
		case 2:
			//FromWord = temp.substr(1, temp.length() - 2);
			break;
		case 3:
			//Really this is FromLemma but I don't want to change variables elsewhere!
			FromWord = temp.substr(1, temp.length() - 2);
			break;
		case 4:
			sscanf(temp.c_str(), "\"%d\"", &ToID);
			break;
		case 5:
			//ToWord = temp.substr(1, temp.length() - 2);
			break;
		case 6:
			//Again, really this is ToLemma
			ToWord = temp.substr(1, temp.length() - 2);
			break;
		case 7:
			Relation = temp.substr(1, temp.length() - 2);
			break;
		}
	}
	std::vector<struct sentence>::iterator sentit;
	sentit = std::find_if(sentences.begin(), sentences.end(), find_sID(sID));
	if (sentit == sentences.end()){
		struct sentence tempsent;
		tempsent.id = sID;
		sentences.emplace_back(tempsent);
		sentit = std::find_if(sentences.begin(), sentences.end(), find_sID(sID));
	}
	int whichword[2];
	whichword[0] = find_word(FromID, (*sentit).words);
	if (whichword[0] == -1){
		struct word tempword;
		tempword.id = FromID;
		tempword.st = FromWord;
		whichword[0] = ((*sentit).words.size());
		(*sentit).words.emplace_back(tempword);
	}
	whichword[1] = find_word(ToID, (*sentit).words);
	if (whichword[1] == -1){
		struct word tempword;
		tempword.id = ToID;
		tempword.st = ToWord;
		whichword[1] = ((*sentit).words.size());
		(*sentit).words.emplace_back(tempword);
	}
	struct link templink;
	templink.from = whichword[0];
	templink.to = whichword[1];
	templink.relat = Relation;
	templink.checked = false;
	(*sentit).links.emplace_back(templink);

	return 1;
}

bool DrInventorFindTriples::isopen(void){
	return fisopen;
}