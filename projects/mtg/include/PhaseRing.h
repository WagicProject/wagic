#ifndef _PHASERING_H_
#define _PHASERING_H_

#include <list>
#include <string>
#include "MTGDefinitions.h"
using namespace std;

/*
 The class that handles the phases of a turn
 */

class Player;
class GameObserver;

typedef enum
{
    BLOCKERS,
    TRIGGERS,
    ORDER,
    FIRST_STRIKE,
    END_FIRST_STRIKE,
    DAMAGE,
    END_DAMAGE
} CombatStep;

class Phase
{
public:
    GamePhase id;
    Player * player;
    bool isExtra;
    Phase(GamePhase id, Player *player, bool _isExtra = false) :
        id(id), player(player), isExtra(_isExtra)
    {
    }
    ;
};

class PhaseRing
{
private:
    bool extraDamagePhase(int id);
    GameObserver* observer;
public:
    list<Phase *> ring;
    list<Phase *>::iterator current;
    list<Phase*>turn;
    list<Phase*>currentTurnList;
    list<Phase*>nextTurnList;
    list<Phase*> currentTurn();
    list<Phase*> nextTurn();
    list<Phase*> extraPhases;
    Phase * getCurrentPhase();
    Phase * forward(bool sendEvents = true);
    Phase * goToPhase(int id, Player * player, bool sendEvents = true);
    Phase * getPhase(int id);
    PhaseRing(GameObserver* observer);
    ~PhaseRing();
    void deleteOldTurn();
    int addPhase(Phase * phase);
    int addCombatAfter(Player* player, int after_id, bool withMain = false);
    int addPhaseAfter(GamePhase id, Player* player, int after_id);
    int removePhase(int id);
    const char * phaseName(int id);
    static GamePhase phaseStrToInt(string s);
    static string phaseIntToStr(int id);

};

#endif
