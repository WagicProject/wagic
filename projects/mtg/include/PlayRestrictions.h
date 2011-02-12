#ifndef _PLAY_RESTRICTIONS_H_
#define _PLAY_RESTRICTIONS_H_


class TargetChooser;
class MTGCardInstance;
class MTGGameZone;

class PlayRestriction 
{

public:
    enum
    {
        LANDS_RULE_ID,
        UNDEF_ID
    };
    
    enum
    {
        CAN_PLAY,
        CANT_PLAY,
        NO_OPINION
    };

    unsigned int id;
    TargetChooser * tc;

    virtual int canPutIntoZone(MTGCardInstance * card, MTGGameZone * destZone) = 0;

    PlayRestriction(unsigned int id, TargetChooser * tc);
    ~PlayRestriction();
};

class MaxPerTurnRestriction: public PlayRestriction
{
public:
    enum
    {
        NO_MAX = -1,
    };
    int maxPerTurn;
    MTGGameZone * zone;
    MaxPerTurnRestriction(unsigned int id, TargetChooser * tc, int maxPerTurn, MTGGameZone * zone);
    int canPutIntoZone(MTGCardInstance * card, MTGGameZone * destZone);
};


class PlayRestrictions
{
protected:
    vector<PlayRestriction *>restrictions;
public:
    PlayRestriction * getRestrictionById(unsigned int id);
    void addRestriction(PlayRestriction * restriction);
    void removeRestriction(PlayRestriction * restriction);
    int canPutIntoZone(MTGCardInstance * card, MTGGameZone * destZone);
    ~PlayRestrictions();

};
#endif