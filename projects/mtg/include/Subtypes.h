#ifndef _SUBTYPES_H_
#define _SUBTYPES_H_


#include <string>
#include <map>
using std::string;
using std::map;

class Subtypes{
protected:
	  int nb_items;
		map<string,int> values;

public:
	static Subtypes * subtypesList;
	Subtypes();
	int Add(const char * subtype);
	int find(const char * subtype);
	int Add(string subtype);
	int find(string subtype);
};


#endif