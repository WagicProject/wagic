#include "../include/config.h"
#include "../include/Translate.h"
#include "../include/GameState.h"
#include "../include/Player.h"
#include "../include/SimpleMenu.h"
#include "../include/DeckStats.h"
#include "../include/DeckMetaData.h"
#include <vector>

int GameState::fillDeckMenu(SimpleMenu * _menu, string path, string smallDeckPrefix, Player * statsPlayer){
  DeckMetaDataList * metas = DeckMetaDataList::decksMetaData;
  int found = 1;
  int nbDecks = 0;
  _menu->autoTranslate = false;
  while (found){
    found = 0;
    char buffer[512];
    char smallDeckName[512];
    char deckDesc[512];
    sprintf(buffer, "%s/deck%i.txt",path.c_str(),nbDecks+1);
    if(DeckMetaData * meta = metas->get(buffer)){
      found = 1;
      nbDecks++;
      sprintf(smallDeckName, "%s_deck%i",smallDeckPrefix.c_str(),nbDecks);
      
      if (statsPlayer){
        DeckStats * stats = DeckStats::GetInstance();
        stats->load(statsPlayer);
        int percentVictories = stats->percentVictories(string(smallDeckName));
        string difficulty;
        if (percentVictories < 34){
          difficulty = "(hard)";
        }else if (percentVictories < 67){
          difficulty = "";
        }else{
          difficulty = "(easy)";
        }
        sprintf(deckDesc, "%s %s",meta->name.c_str(), _(difficulty).c_str());
      }else{
        sprintf(deckDesc, "%s",meta->name.c_str());
      }
      deckDesc[16] = 0;
      //translate decks desc
      Translator * t = Translator::GetInstance();
      map<string,string>::iterator it = t->deckValues.find(meta->name);
      if (it != t->deckValues.end())
        _menu->Add(nbDecks,deckDesc,it->second);
      else
        _menu->Add(nbDecks,deckDesc,meta->desc);
    }
  }
  return nbDecks;
}

int GameState::fillDeckMenu(vector<int> * deckIdList, SimpleMenu * _menu, string path, string smallDeckPrefix, Player * statsPlayer){
  DeckMetaDataList * metas = DeckMetaDataList::decksMetaData;
  int found = 1;
  int nbDecks = 0;
  _menu->autoTranslate = false;
  map<string,DeckMetaData> menu;
  list<string> deckNameVector;
  while (found){
    found = 0;
    char buffer[512];
    char smallDeckName[512];
    char deckDesc[512];
    sprintf(buffer, "%s/deck%i.txt",path.c_str(),nbDecks+1);
    if(DeckMetaData * meta = metas->get(buffer)){
      found = 1;
      nbDecks++;
      sprintf(smallDeckName, "%s_deck%i",smallDeckPrefix.c_str(),nbDecks);
      
      if (statsPlayer){
        DeckStats * stats = DeckStats::GetInstance();
        stats->load(statsPlayer);
        int percentVictories = stats->percentVictories(string(smallDeckName));
        string difficulty;
        if (percentVictories < 34){
          difficulty = "(hard)";
        }else if (percentVictories < 67){
          difficulty = "";
        }else{
          difficulty = "(easy)";
        }
        sprintf(deckDesc, "%s %s",meta->name.c_str(), _(difficulty).c_str());
      }else{
        sprintf(deckDesc, "%s",meta->name.c_str());
      }
      deckDesc[16] = 0;
      //translate decks desc
      Translator * t = Translator::GetInstance();
      map<string,string>::iterator it = t->deckValues.find(meta->name);
      if (it != t->deckValues.end())
        _menu->Add(nbDecks,deckDesc, it->second);
      else
      {
        menu[deckDesc] = *meta;
        deckNameVector.push_back( deckDesc );        
      }
    }
  }
  
    deckNameVector.sort();
    int deckNumber = 1;
    deckIdList->clear();
    
    for (list<string>::iterator i = deckNameVector.begin(); i != deckNameVector.end(); i++)
    {
        string deckName = *i;
        DeckMetaData meta = menu[ deckName ];
        string deckDescription = meta.desc;
        deckIdList->push_back( meta.deckid );
        _menu->Add( deckNumber++ ,deckName.c_str(), deckDescription.c_str());
    }
  return nbDecks;
}

