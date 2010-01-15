#ifndef _DECKMETADATA_H_
#define _DECKMETADATA_H_

#include <string>
#include <map>

using namespace std;

class DeckMetaData {
public:
  DeckMetaData(string filename);
  void load(string filename);
  string desc;
  string name;
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