#ifndef _DECKMETADATA_H_
#define _DECKMETADATA_H_

#include <string>
#include <vector>
#include <map>
#include "../include/DeckStats.h"


using namespace std;
enum DECK_DIFFICULTY
{
    HARD = -1,
    NORMAL = 0,
    EASY = 1
};
    
class DeckMetaData {
public:
  DeckMetaData();
  DeckMetaData(string filename, Player * statsPlayer);
  void load(string filename);
  void loadStatsForPlayer( Player * statsPlayer, string opponentDeckName = "" );
  
  string getDescription();
  
  string desc;
  string name;
  int deckid;

  // statistical information
  
  int nbGamesPlayed, victories, percentVictories, difficulty;
};

class DeckMetaDataList {
public:
  void invalidate(string filename);
  DeckMetaData * get(string filename, Player * statsPlayer = NULL);
  ~DeckMetaDataList();
  static DeckMetaDataList * decksMetaData;

  
private:
  map<string,DeckMetaData *>values;
};

#endif
