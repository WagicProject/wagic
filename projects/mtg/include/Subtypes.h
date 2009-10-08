#ifndef _SUBTYPES_H_
#define _SUBTYPES_H_


#include <string>
#include <map>
using std::string;
using std::map;

class Subtypes{
 protected:
  map<string,int> values;
  map<int,string> valuesById;
  int nb_items;
 public:
  static Subtypes * subtypesList;
  Subtypes();
  int Add(const char * subtype);
  int find(const char * subtype);
  int Add(string subtype);
  int find(string subtype);
  string find(int id);
};


#endif
