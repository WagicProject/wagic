#include "../include/config.h"
#include "../include/Subtypes.h"
#include <JGE.h>
#include <algorithm>

Subtypes * Subtypes::subtypesList = NEW Subtypes();


Subtypes::Subtypes(){
  //Add the more common types, so that they can be accessed through ints
  //these should be added in the same order as the enum defined in subtypes.h!!!
  find("Creature");
  find("Enchantment");
  find("Sorcery");
  find("Instant");
  find("Land");
  find("Artifact");
}

int Subtypes::find(string value, bool forceAdd){
  if (value[0]>=97 && value[0]<=122) value[0]-=32; //Poor man's camelcase. We assume strings we get are either Camelcased or lowercase
  map<string,int>::iterator it = values.find(value);
  if (it != values.end()) return it->second;
  if (!forceAdd) return 0;
  int id = (int)(valuesById.size() + 1);
  values[value] = id;
  valuesById.push_back(value);
  return id;
}

int Subtypes::find(const char * subtype, bool forceAdd){
  string value = subtype;
  return (find(value));
}

string Subtypes::find(unsigned int id){
  if (valuesById.size() < id || !id) return "";
  return valuesById[id - 1];
}
