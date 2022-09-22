#include <cctype>
#include <iostream>
#include <fstream>
#include <ios>
#include <list>

#include "error.h"

class XMLData {
public:
	XMLData* find(std::string name);
	std::string& getValue();
	void write(std::ofstream& out, int tab);
	void add(XMLData* node);
	XMLData(char* name, char* value);
	XMLData(char* filename) throw (ReadError);
	~XMLData();
private:
	XMLData(std::string* name, FILE* f);
	std::string* readToken();
	int get();
	void unget(int c);
	void seek(long pos);
	void read();
	std::string* getName(std::string* token);
	FILE* file;
	std::string* name;
	std::string value;
	std::list<XMLData*> nodes;
	bool debug;
	int ungot;
	bool ungotReady;
};

