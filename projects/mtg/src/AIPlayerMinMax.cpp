#include "PrecompiledHeader.h"

#include "AIPlayerMinMax.h"
#include "CardDescriptor.h"
#include "AIStats.h"
#include "AllAbilities.h"
#include "ExtraCost.h"
#include "GuiCombat.h"
#include "AIHints.h"
#include "ManaCostHybrid.h"
#include "MTGRules.h"

namespace AI {

//
// Abilities/Target Selection
//

AIPlayerMinMax::AIPlayerMinMax(GameObserver *observer, string deckFile, string deckFileSmall, string avatarFile, MTGDeck * deck) :
    AIPlayer(observer, deckFile, deckFileSmall, avatarFile, deck)
{
}

int AIPlayerMinMax::Act(float dt)
{
  return 0;
};

AIPlayerMinMax::~AIPlayerMinMax() 
{
}


void AIPlayerMinMax::LookAround() 
{
    vector<MTGCardInstance*>::iterator ite;
    vector<AIAction> potentialActions;

    // look for something useable (including mana)
    for (size_t i = 1; i < observer->mLayers->actionLayer()->mObjects.size(); i++)
    {
        MTGAbility * a = ((MTGAbility *) observer->mLayers->actionLayer()->mObjects[i]);
        //Make sure we can use the ability
        for (int j = 0; j < game->inPlay->nb_cards; j++)
        {
            MTGCardInstance * card = game->inPlay->cards[j];
            if (a->isReactingToClick(card, 0))
            { 
                createAbilityPotentialsActions(a, card, potentialActions);
            }
        }
    }

    // look for something playable
    for(ite = game->hand->cards.begin(); ite != game->hand->cards.end(); ite++)
    {
        if(canPlay(*ite))
        {
          AIAction a(this, (*ite));
          potentialActions.push_back(a);
        }
    }
    
    stringstream stream;
    stream << *observer;
    vector<AIAction>::iterator it;
    for(it = potentialActions.begin(); it != potentialActions.end(); it++)
    {
        GameObserver g;
        g.load(stream.str());
//        g.processAction((*it));
    }
}

}
