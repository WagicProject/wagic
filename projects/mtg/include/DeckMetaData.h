#ifndef _DECKMETADATA_H_
#define _DECKMETADATA_H_

#include <string>
#include <vector>
#include <map>

using namespace std;

class DeckMetaData {
public:
  DeckMetaData();
  DeckMetaData(string filename);
  void load(string filename);
  bool operator<(DeckMetaData b);
  
  string desc;
  string name;
  int deckid;
  
      string& trim(string &str);
    string& ltrim(string &str);
    string& rtrim(string &str);
};

class DeckMetaDataList {
public:
  void invalidate(string filename);
  DeckMetaData * get(string filename);
  ~DeckMetaDataList();
  static DeckMetaDataList * decksMetaData;

  
private:
  map<string,DeckMetaData *>values;
};

#endif
