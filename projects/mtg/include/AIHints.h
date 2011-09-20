#ifndef _AIHINTS_H_
#define _AIHINTS_H_

#include <string>
#include <vector>
using std::string;
using std::vector;

#include "AIPlayerBaka.h"

class ManaCost;
class MTGAbility;

class AIHint
{
public:
    string mCondition;
    string mAction;
    int mSourceId;
    AIHint(string line);
};


class AIHints
{
protected:
    AIPlayerBaka * mPlayer;
    vector<AIHint *> hints;
    AIHint * getByCondition (string condition);
    AIAction * findAbilityRecursive(AIHint * hint, ManaCost * potentialMana);
    vector<MTGAbility *> findAbilities(AIHint * hint);
    RankingContainer findActions(AIHint * hint);
    string constraintsNotFulfilled(AIAction * a, AIHint * hint, ManaCost * potentialMana);
    bool findSource(int sourceId);
    bool abilityMatches(MTGAbility * a, AIHint * hint);
public:
    AIHints (AIPlayerBaka * player);
    AIAction * suggestAbility(ManaCost * potentialMana);
    void add(string line);
    ~AIHints();
};

#endif
