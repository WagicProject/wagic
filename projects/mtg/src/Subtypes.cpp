#include "../include/config.h"
#include "../include/Subtypes.h"
#include <JGE.h>
#include <algorithm>

Subtypes * Subtypes::subtypesList = NEW Subtypes();



Subtypes::Subtypes(){
  nb_items = 100;
}

int Subtypes::Add(string value){
  int result = find(value);
  if (result) return result;
  std::transform( value.begin(), value.end(), value.begin(), ::tolower );
  nb_items++;
  values[value] = nb_items;
  valuesById[nb_items] = value;
  return nb_items;
}

int Subtypes::Add(const char * subtype){
  string value = subtype;
  return Add(value);

}

int Subtypes::find(string value){
  std::transform( value.begin(), value.end(), value.begin(), ::tolower );
  map<string,int>::iterator it = values.find(value);
  if (it != values.end()) return it->second;
  return 0;
}

int Subtypes::find(const char * subtype){
  string value = subtype;
  return (find(value));

}

string Subtypes::find(int id){
  map<int,string>::iterator it=valuesById.find(id);;
  if (it != valuesById.end()) return it->second;
  return "";
}
