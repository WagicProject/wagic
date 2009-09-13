#ifndef _PHASERING_H_
#define _PHASERING_H_

#include <list>
using std::list;

/*
  The class that handles the phases of a turn
*/

class Player;

typedef enum { BLOCKERS, ORDER, FIRST_STRIKE, END_FIRST_STRIKE, DAMAGE, END_DAMAGE } CombatStep;
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
  Phase * forward();
  Phase * goToPhase(int id, Player * player);
  PhaseRing(Player* players[], int nbPlayers=2);
  ~PhaseRing();
  int addPhase(Phase * phase);
  int addPhaseBefore(int id, Player* player,int after_id, Player * after_player, int allOccurences = 1);
  int removePhase (int id, Player * player, int allOccurences = 1);
  static const char *  phaseName(int id);
  
};

#endif
