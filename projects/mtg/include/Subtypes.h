#ifndef _SUBTYPES_H_
#define _SUBTYPES_H_

#define MAX_SUBTYPES 1000

#include <string>
using std::string;

class Subtypes{
protected:
		int nb_items;
		string values[MAX_SUBTYPES];

public:
	static Subtypes * subtypesList;
	Subtypes();
	int offset;
	int Add(const char * subtype);
	int find(const char * subtype);
	int Add(string subtype);
	int find(string subtype);
};


#endif