#ifndef _AISTATS_H_
#define _AISTATS_H_

#define STATS_PLAYER_MULTIPLIER 15
#define STATS_CREATURE_MULTIPLIER 10

//floats
#define STATS_AURA_MULTIPLIER 0.9
#define STATS_LORD_MULTIPLIER 0.5

#include <list>
#include <string>
using std::list;
using std::string;
class Player;
class MTGCardInstance;
class MTGCard;
class Damage;
class WEvent;

class AIStat{
 public:
  int source; //MTGId of the card
  int value;
  int occurences;
  bool direct;
 AIStat(int _source, int _value, int _occurences, bool _direct):source(_source), value(_value),occurences(_occurences),direct(_direct){};
};



class AIStats{
 public:
  Player * player;
  string filename;
  list<AIStat *> stats;
  AIStats(Player * _player, char * filename);
  ~AIStats();
  void load(char * filename);
  void save();
  AIStat * find(MTGCard * card);
  bool isInTop(MTGCardInstance * card, unsigned int max, bool tooSmallCountsForTrue = true );
  void updateStatsCard(MTGCardInstance * cardInstance, Damage * damage, float multiplier = 1.0);
  int receiveEvent(WEvent * event);
  void Render();
};

#endif
