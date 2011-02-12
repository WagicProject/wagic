#include "PrecompiledHeader.h"

#include "playRestrictions.h"
#include "TargetChooser.h"
#include "MTGCardInstance.h"


PlayRestriction::PlayRestriction(unsigned int id, TargetChooser * tc): id(id), tc(tc)
{
    tc->setAllZones(); // This is to allow targetting cards without caring about the actual zone
    tc->targetter = NULL;
};

PlayRestriction::~PlayRestriction()
{
    SAFE_DELETE(tc);
};


MaxPerTurnRestriction::MaxPerTurnRestriction(unsigned int id, TargetChooser * tc, int maxPerTurn, MTGGameZone * zone): 
    PlayRestriction(id, tc), maxPerTurn(maxPerTurn), zone(zone)
    {}

int  MaxPerTurnRestriction::canPutIntoZone(MTGCardInstance * card, MTGGameZone * destZone)
{
    if (destZone != zone)
        return PlayRestriction::NO_OPINION;

    if (!tc->canTarget(card))
        return PlayRestriction::NO_OPINION;

    if (maxPerTurn == NO_MAX) return PlayRestriction::CAN_PLAY;

    if (zone->seenThisTurn(tc) >= maxPerTurn)
        return PlayRestriction::CANT_PLAY;

    return PlayRestriction::CAN_PLAY;
};


PlayRestriction * PlayRestrictions::getRestrictionById(unsigned int id)
{
    if (id == PlayRestriction::UNDEF_ID)
        return NULL; // do not request Restrictions that don't have an id, there are several of them

    for (vector<PlayRestriction *>::iterator iter = restrictions.begin(); iter != restrictions.end(); ++iter)
    {
        if ((*iter)->id == id)
            return *iter;
    }

    return NULL;
}

void PlayRestrictions::addRestriction(PlayRestriction * restriction)
{
    //TODO control that the id does not already exist?
    restrictions.push_back(restriction);

}

void PlayRestrictions::removeRestriction(PlayRestriction * restriction)
{
    for (vector<PlayRestriction *>::iterator iter = restrictions.begin(); iter != restrictions.end(); ++iter)
    {
        if(*iter == restriction)
        {
            restrictions.erase(iter);
            return;
        }
    }
}

int  PlayRestrictions::canPutIntoZone(MTGCardInstance * card, MTGGameZone * destZone)
{
    if (!card)
        return PlayRestriction::CANT_PLAY;

    for (vector<PlayRestriction *>::iterator iter = restrictions.begin(); iter != restrictions.end(); ++iter)
    {
            if ((*iter)->canPutIntoZone(card, destZone) == PlayRestriction::CANT_PLAY)
                return PlayRestriction::CANT_PLAY;
    }

    return PlayRestriction::CAN_PLAY;
}

PlayRestrictions::~PlayRestrictions()
{
    for (vector<PlayRestriction *>::iterator iter = restrictions.begin(); iter != restrictions.end(); ++iter)
    {
        SAFE_DELETE(*iter);
    }
    restrictions.clear();
}
