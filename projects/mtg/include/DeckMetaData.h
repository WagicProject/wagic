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
private:
  string _filename;

  string _desc;
  string _name;
  int _deckid;

  // statistical information
  
  int _nbGamesPlayed, _victories, _percentVictories, _difficulty;

public:
  DeckMetaData();
  DeckMetaData(string filename, Player * statsPlayer);
  void load(string filename);
  void loadStatsForPlayer( Player * statsPlayer, string opponentDeckName = "" );
  
  // Accessors
  string getFilename();
  string getDescription();
  string getName();
  int getDeckId();
  int getGamesPlayed();
  int getVictories();
  int getVictoryPercentage();
  int getDifficulty();

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
