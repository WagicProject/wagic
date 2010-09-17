#include "../include/config.h"
#include "../include/Translate.h"
#include "../include/GameState.h"
#include "../include/Player.h"
#include "../include/SimpleMenu.h"
#include "../include/DeckStats.h"
#include "../include/DeckMetaData.h"
#include "../include/Player.h"
#include <vector>

// The purpose of this method is to create a listing of decks to be used for the input menu
// by default, the list will be sorted by name
// TODO: revise sorting strategy to allow other types of sorting.  Currently, it is hardwired to use
//    sortByName to do the sorting.  This was done since the menu item display is done in insertion order.

vector<DeckMetaData *> GameState::fillDeckMenu( SimpleMenu * _menu, string path, string smallDeckPrefix, Player * statsPlayer){
  _menu->autoTranslate = false;
  vector<DeckMetaData *> deckMetaDataVector = getValidDeckMetaData( path, smallDeckPrefix, statsPlayer );
  renderDeckMenu( _menu, deckMetaDataVector);
  
  return deckMetaDataVector;
}


vector<DeckMetaData *> GameState::getValidDeckMetaData( string path, string smallDeckPrefix, Player * statsPlayer)
{
  vector<DeckMetaData*> retList;
  
  DeckMetaDataList * metas = DeckMetaDataList::decksMetaData;
  int found = 1;
  int nbDecks = 1;
  while (found){
    found = 0;
    char buffer[512];
    char smallDeckName[512];
    char deckDesc[512];
    sprintf(buffer, "%s/deck%i.txt",path.c_str(),nbDecks);
    if(DeckMetaData * meta = metas->get(buffer, statsPlayer)){
      found = 1;
      sprintf(smallDeckName, "%s_deck%i",smallDeckPrefix.c_str(),nbDecks);
      sprintf(deckDesc, "%s",meta->name.c_str());

      if (statsPlayer){
        string smallDeckNameStr = string(smallDeckName);
        meta->loadStatsForPlayer( statsPlayer, smallDeckNameStr );
      }
      else
      {
        char playerStatsDeckName[512];
        
        sprintf(playerStatsDeckName, "stats/player_deck%i.txt", nbDecks);
        string deckstats = options.profileFile(playerStatsDeckName);
        meta->loadStatsForPlayer( NULL, deckstats );
      }
      
      deckDesc[16] = 0;
      retList.push_back( meta );
      nbDecks++;
    }
  }

  std::sort( retList.begin(), retList.end(), sortByName);

  return retList;

}


// build a menu with the given deck list and return a vector of the deck ids created.
void GameState::renderDeckMenu ( SimpleMenu * _menu, vector<DeckMetaData *> deckMetaDataList )
{
  int deckNumber = 1;
  Translator * t = Translator::GetInstance();
  map<string,string>::iterator it;
  for (vector<DeckMetaData *>::iterator i = deckMetaDataList.begin(); i != deckMetaDataList.end(); i++)
  {
    DeckMetaData * deckMetaData = *i;
    string deckName = deckMetaData -> name;
    string deckDescription = deckMetaData -> getDescription();
    //translate decks desc
    it = t->deckValues.find(deckName);
    if (it != t->deckValues.end())
      _menu->Add(deckNumber++, deckName.c_str(), it->second);
    else
      _menu->Add( deckNumber++ ,deckName.c_str(), deckDescription.c_str());
  }
}


// deck sorting routines
bool sortByName( DeckMetaData * d1, DeckMetaData * d2 )
{
  return strcmp( d1->name.c_str(), d2->name.c_str()) < 0;

}


//end deck sorting routine
