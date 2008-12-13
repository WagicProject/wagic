#include "../include/debug.h"
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
#if defined (WIN32) || defined (LINUX)
  char buf[4096];
  sprintf(buf, "Adding new type: *%s*\n",value.c_str());
  OutputDebugString(buf);
#endif
  std::transform( value.begin(), value.end(), value.begin(), ::tolower );
  nb_items++;
  values[value] = nb_items;
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

/*This will be slow... */
string Subtypes::find(int id){
  map<string,int>::iterator it;
  for (it = values.begin(); it != values.end(); it++){
    if (it->second == id) return it->first;
  }
  return NULL;
}
