#include "xml.h"
#include <cstring>

XMLData::XMLData(std::string* name, FILE* f) :
	file(f),
	debug(false),
	ungot(-1),
	ungotReady(false)
{
	this->name = new std::string(name->c_str());
}

XMLData::XMLData(char* filename) throw (ReadError) :
	debug(false),
	ungot(-1),
	ungotReady(false)

{
	name = new std::string("main");

	file = fopen(filename, "r");
	if (!file) {
		delete name;
		throw new ReadError();
	}

	read();

	fclose(file);
}

XMLData::XMLData(char* name, char* value) :
	debug(false)
{
	this->name = new std::string(name);
	this->value = std::string(value);
}

void XMLData::add(XMLData* node)
{
	nodes.push_back(node);
}

XMLData* XMLData::find(std::string name)
{
	std::list<XMLData*>::iterator it = nodes.begin();

	while (it != nodes.end()) {
		if (*(*it)->name == name) {
			return *it;
		}
		it++;
	}

	return 0;
}

std::string& XMLData::getValue()
{
	return value;
}

XMLData::~XMLData()
{
	std::list<XMLData*>::iterator it = nodes.begin();

	while (it != nodes.end()) {
		XMLData* node = *it;
		delete node;
		it++;
	}

	delete name;
}

std::string* XMLData::readToken()
{
	if (feof(file)) {
		return 0;
	}

	int c;

	/* Skip whitespace */

	for (;;) {
		c = get();
		if (c == EOF) {
			return 0;
		}
		if (!isspace(c)) {
			break;
		}
	}

	/* Found tag */

	if (c == '<') {
		if (debug)
			std::cout << "Found tag start/end\n";
		std::string token;
		token += c;
		for (;;) {
			c = get();
			if (c == EOF) {
				break;
			}
			token += c;
			if (c == '>')
				break;
		}
		if (debug)
			std::cout << "Read <token>: " << token << "\n";
		return new std::string(token);
	}
	/* Found data */
	else {
		std::string data;
		data += c;
		for (;;) {
			c = get();
			if (c == EOF) {
				break;
			}
			if (c == '<') {
				unget(c);
				break;
			}
			data += c;
		}
		if (debug)
			std::cout << "Read data: " << data << "\n";
		return new std::string(data);
	}

	return 0;
}

int XMLData::get()
{
	int c;

	if (ungotReady) {
		c = ungot;
		ungotReady = false;
	}
	else {
		c = fgetc(file);
	}

	return c;
}

void XMLData::unget(int c)
{
	ungot = c;
	ungotReady = true;
}

void XMLData::read()
{
	// read until EOF or end token
	
	for (;;) {
		std::string* token;
		token = readToken();
		if (!token || (token && !strncmp(token->c_str(), "</", 2))) {
			if (token) {
				delete token;
			}
			return;
		}
		if (token->c_str()[0] == '<') {
			if (debug) {
				std::cout << "Reading sub tag\n";
			}
			std::string* name = getName(token);
			if (debug)
				std::cout << "Token is " << name->c_str() << "\n";
			XMLData* newdata = new XMLData(name, file);
			delete name;
			newdata->read();
			nodes.push_back(newdata);
		}
		else {
			value += token->c_str();
		}
		delete token;
	}
}

std::string* XMLData::getName(std::string* token)
{
	std::string* s = new std::string;
	
	for (int i = 1; token->c_str()[i] != '>' && token->c_str()[i]; i++) {
		if (debug)
			std::cout << "Read character " << i << "\n";
		*s += token->c_str()[i];
	}

	return s;
}

void XMLData::write(std::ofstream& out, int tabs = 0)
{
	if (value == "") {
		for (int i = 0; i < tabs; i++) {
			out << "\t";
		}

		out << "<" << name->c_str() << ">" << std::endl;

		std::list<XMLData*>::iterator it = nodes.begin();

		while (it != nodes.end()) {
			XMLData* node = dynamic_cast<XMLData*>(*it);
			node->write(out, tabs+1);
			it++;
		}

		for (int i = 0; i < tabs; i++) {
			out << "\t";
		}

		out << "</" << name->c_str() << ">" << std::endl;
	}
	else {
		for (int i = 0; i < tabs; i++) {
			out << "\t";
		}

		out << "<" << name->c_str() << ">";

		out << value;
		
		std::list<XMLData*>::iterator it = nodes.begin();

		while (it != nodes.end()) {
			XMLData* node = dynamic_cast<XMLData*>(*it);
			node->write(out, tabs+1);
			it++;
		}

		out << "</" << name->c_str() << ">" << std::endl;
	}
}

/*
void XMLData::write(std::ofstream& out, int tabs = 0)
{
	if (value == "") {
		out << "<" << name->c_str() << ">";

		std::list<XMLData*>::iterator it = nodes.begin();

		while (it != nodes.end()) {
			XMLData* node = dynamic_cast<XMLData*>(*it);
			node->write(out, tabs+1);
			it++;
		}

		out << "</" << name->c_str() << ">";
	}
	else {
		out << "<" << name->c_str() << ">";

		out << value;
		
		std::list<XMLData*>::iterator it = nodes.begin();

		while (it != nodes.end()) {
			XMLData* node = dynamic_cast<XMLData*>(*it);
			node->write(out, tabs+1);
			it++;
		}

		out << "</" << name->c_str() << ">";
	}
}
*/
