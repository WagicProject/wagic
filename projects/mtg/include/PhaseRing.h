#ifndef _PHASERING_H_
#define _PHASERING_H_

#include <list>
#include <string>
using namespace std;

/*
  The class that handles the phases of a turn
*/

class Player;

typedef enum { BLOCKERS, TRIGGERS, ORDER, FIRST_STRIKE, END_FIRST_STRIKE, DAMAGE, END_DAMAGE } CombatStep;
class Phase{
 public:
  int id;
  Player * player;
 Phase(int id, Player *player):id(id),player(player){};
};

class PhaseRing{
private:
  static bool extraDamagePhase(int id);
 public:
  list<Phase *> ring;
  list<Phase *>::iterator current;
  Phase * getCurrentPhase();
  Phase * forward(bool sendEvents = true);
  Phase * goToPhase(int id, Player * player,bool sendEvents = true);
  PhaseRing(Player* players[], int nbPlayers=2);
  ~PhaseRing();
  int addPhase(Phase * phase);
  int addPhaseBefore(int id, Player* player,int after_id, Player * after_player, int allOccurences = 1);
  int removePhase (int id, Player * player, int allOccurences = 1);
  static const char *  phaseName(int id);
  static int phaseStrToInt(string s);
  
};

#endif
