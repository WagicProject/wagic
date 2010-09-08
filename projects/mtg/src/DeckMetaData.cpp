#include "../include/DeckMetaData.h"
#include "../include/MTGDeck.h"
#include "../include/config.h"
//Possible improvements:
//Merge this with DeckStats
//Have this class handle all the Meta Data rather than relying on MTGDeck. Then MTGDeck would have a MetaData object...


DeckMetaDataList * DeckMetaDataList::decksMetaData = NEW DeckMetaDataList();

DeckMetaData::DeckMetaData(){
  
}

DeckMetaData::DeckMetaData(string filename){
  load(filename);
}

void DeckMetaData::load(string filename){
  MTGDeck * mtgd = NEW MTGDeck(filename.c_str(),NULL,1);
  name = DeckMetaData::trim( mtgd->meta_name );
  desc =  DeckMetaData::trim( mtgd->meta_desc );
  deckid = atoi( (filename.substr( filename.find("deck") + 4, filename.find(".txt") )).c_str() );
  delete(mtgd);
}


// Must define less than relative to DeckMetaData objects.
bool DeckMetaData::operator<(DeckMetaData b)
{
    return strcmp(name.c_str(), b.name.c_str()) < 0;
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



string& DeckMetaData::trim(string &str)
{
    int i,j,start,end;

    //ltrim
    for (i=0; (str[i]!=0 && str[i]<=32); )
        i++;
    start=i;

    //rtrim
    for(i=0,j=0; str[i]!=0; i++)
        j = ((str[i]<=32)? j+1 : 0);
    end=i-j;
    str = str.substr(start,end-start);
    return str;
}


string& DeckMetaData::ltrim(string &str)
{
    int i,start;

    for (i=0; (str[i]!=0 && str[i]<=32); )
        i++;
    start=i;

    str = str.substr(start,str.length()-start);
    return str;
}
string& DeckMetaData::rtrim(string &str)
{
    int i,j,end;

    for(i=0,j=0; str[i]!=0; i++)
        j = ((str[i]<=32)? j+1 : 0);
    end=i-j;

    str = str.substr(0,end);
    return str;
}
