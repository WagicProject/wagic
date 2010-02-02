#include "../include/DeckMetaData.h"
#include "../include/MTGDeck.h"
#include "../include/config.h"
//Possible improvements:
//Merge this with DeckStats
//Have this class handle all the Meta Data rather than relying on MTGDeck. Then MTGDeck would have a MetaData object...


DeckMetaDataList * DeckMetaDataList::decksMetaData = NEW DeckMetaDataList();

DeckMetaData::DeckMetaData(string filename){
  load(filename);
}
void DeckMetaData::load(string filename){
  MTGDeck * mtgd = NEW MTGDeck(filename.c_str(),NULL,1);
  name = mtgd->meta_name;
  desc = mtgd->meta_desc;
  delete(mtgd);
}

DeckMetaDataList::~DeckMetaDataList(){
  for(map<string,DeckMetaData *>::iterator it = values.begin(); it != values.end(); ++it){
    SAFE_DELETE(it->second);
  }
  values.clear();
}

void DeckMetaDataList::invalidate(string filename){
   map<string,DeckMetaData *>::iterator it = values.find(filename);
   if (it !=values.end()){
     SAFE_DELETE(it->second);
     values.erase(it);
   }
}

DeckMetaData * DeckMetaDataList::get(string filename){
  map<string,DeckMetaData *>::iterator it = values.find(filename);
  if (it ==values.end()){
    if (fileExists(filename.c_str())) {
      values[filename] = NEW DeckMetaData(filename);
    }
  }

   return values[filename]; //this creates a NULL entry if the file does not exist
}
