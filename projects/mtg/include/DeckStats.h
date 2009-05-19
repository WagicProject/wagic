#ifndef _DECKSTATS_H_
#define _DECKSTATS_H_


#include <map>
#include <string>
using namespace std;

class Player;
class GameObserver;

class DeckStat{
public:
  int nbgames;
  int victories;
  DeckStat(int _nbgames = 0 , int _victories = 0):nbgames(_nbgames),victories(_victories){};
  int percentVictories();
};

class DeckStats{
protected:
  static DeckStats * mInstance;
public:
  map<string, DeckStat *>stats; 
  static DeckStats * GetInstance();
  void saveStats(Player * player, Player * opponent, GameObserver * game);
  void save(const char * filename);
  void save(Player * player);
  void load(const char * filename);
  void load(Player * player);
  void cleanStats();
  ~DeckStats();
  int percentVictories(string opponentsDeckFile);
  int percentVictories();
  int nbGames();
};

#endif
