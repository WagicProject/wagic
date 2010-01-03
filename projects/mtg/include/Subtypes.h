#ifndef _SUBTYPES_H_
#define _SUBTYPES_H_


#include <string>
#include <map>
#include <vector>
using namespace std;


class Subtypes{
public:
  //A list of commonly used types
  enum {
    TYPE_CREATURE = 1,
    TYPE_ENCHANTMENT = 2,
    TYPE_SORCERY = 3,
    TYPE_INSTANT = 4,
    TYPE_LAND = 5,
    TYPE_ARTIFACT = 6,
    TYPE_LEGENDARY = 7,
  };


 protected:
  map<string,int> values;
  vector<string> valuesById;
 public:
  static Subtypes * subtypesList;
  Subtypes();
  int find(const char * subtype, bool forceAdd = true);
  int find(string subtype, bool forceAdd = true);
  string find(unsigned int id);
};


#endif
