#ifndef _TARGETCHOOSER_H_
#define _TARGETCHOOSER_H_

#define TARGET_NOK 0
#define TARGET_OK 1
#define TARGET_OK_FULL 2
#define TARGET_OK_NOT_READY 3

#include <JGE.h>
#include "TargetsList.h"
#include "ActionStack.h"

#include <string>
using std::string;

class MTGCardInstance;
class MTGGameZone;
class Player;
class Damageable;
class Targetable;
class CardDescriptor;

class TargetChooser: public TargetsList
{
protected:
    int forceTargetListReady;
public:
    const static int UNLITMITED_TARGETS = 1000;
    enum
    {
        UNSET = 0,
        OPPONENT = -1,
        CONTROLLER = 1,
        TARGET_CONTROLLER = 2,
        OWNER = 3,
        TARGETED_PLAYER = 4
    };
    bool other;
    bool withoutProtections;
    TargetChooser(GameObserver *observer, MTGCardInstance * card = NULL, int _maxtargets = UNLITMITED_TARGETS, bool other = false, bool targetMin = false);
    Player * Owner;
    GameObserver *observer;
    MTGCardInstance * source;
    MTGCardInstance * targetter; //Optional, usually equals source, used for protection from...
    int maxtargets;
    bool done;
    bool targetMin;
    bool validTargetsExist(int maxTarget = 1);
    int attemptsToFill;
    string belongsToAbility;
    int countValidTargets();
    virtual int setAllZones()
    {
        return 0;
    }
    virtual bool targetsZone(MTGGameZone * z)
    {
        return false;
    }
    virtual bool targetsZone(MTGGameZone * z,MTGCardInstance * mSource)
    {
        return false;
    }
    ;
    int ForceTargetListReady();
    int targetsReadyCheck();
    virtual int addTarget(Targetable * target);
    virtual bool canTarget(Targetable * _target,bool withoutProtections = false);

    //returns true if tc is equivalent to this TargetChooser
    //Two targetchoosers are equivalent if they target exactly the same cards
    virtual bool equals(TargetChooser * tc);

    virtual int full()
    {
        if ( (maxtargets != UNLITMITED_TARGETS && (int(targets.size())) >= maxtargets) || done)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    ;
    virtual int ready()
    {
        return (int) (targets.size());
    }
    ;
    virtual ~TargetChooser()
    {
    }
    ;
    int targetListSet();
    virtual TargetChooser* clone() const = 0;
};

class TargetChooserFactory
{
    GameObserver* observer;
public:
    TargetChooserFactory(GameObserver *observer) : observer(observer) {};
    TargetChooser * createTargetChooser(string s, MTGCardInstance * card, MTGAbility * ability = NULL);
    TargetChooser * createTargetChooser(MTGCardInstance * card);
};

class TargetZoneChooser: public TargetChooser
{
public:
    int zones[15];
    int nbzones;
    int init(int * _zones, int _nbzones);
    bool targetsZone(MTGGameZone * z);
    bool targetsZone(MTGGameZone * z,MTGCardInstance * mSource);
    bool withoutProtections;
    TargetZoneChooser(GameObserver *observer, MTGCardInstance * card = NULL, int _maxtargets = 1, bool other = false, bool targetMin = false);
    TargetZoneChooser(GameObserver *observer, int * _zones, int _nbzones, MTGCardInstance * card = NULL, int _maxtargets = 1, bool other = false, bool targetMin = false);
    virtual bool canTarget(Targetable * _card,bool withoutProtections = false);
    int setAllZones();
    virtual TargetZoneChooser * clone() const;
    virtual bool equals(TargetChooser * tc);
};

class CardTargetChooser: public TargetZoneChooser
{
protected:
    MTGCardInstance * validTarget;
public:
    bool withoutProtections;
    CardTargetChooser(GameObserver *observer, MTGCardInstance * card, MTGCardInstance * source, int * zones = NULL, int nbzones = 0);
    virtual bool canTarget(Targetable * target,bool withoutProtections = false);
    virtual CardTargetChooser * clone() const;
    virtual bool equals(TargetChooser * tc);
};

class TypeTargetChooser: public TargetZoneChooser
{
public:
    int nbtypes;
    int types[10];
    bool withoutProtections;
    TypeTargetChooser(GameObserver *observer, const char * _type, MTGCardInstance * card = NULL, int _maxtargets = 1, bool other = false, bool targetMin = false);
    TypeTargetChooser(GameObserver *observer, const char * _type, int * _zones, int nbzones, MTGCardInstance * card = NULL, int _maxtargets = 1, bool other = false, bool targetMin = false);
    void addType(int type);
    void addType(const char * type);
    virtual bool canTarget(Targetable * target,bool withoutProtections = false);
    virtual TypeTargetChooser * clone() const;
    virtual bool equals(TargetChooser * tc);
};

class DamageableTargetChooser: public TypeTargetChooser
{
public:
    bool withoutProtections;
    DamageableTargetChooser(GameObserver *observer, int * _zones, int _nbzones, MTGCardInstance * card = NULL, int _maxtargets = 1, bool other = false, bool targetMin = false) :
        TypeTargetChooser(observer, "creature",_zones, _nbzones, card, _maxtargets, other, targetMin)
    {
    }
    ;
    DamageableTargetChooser(GameObserver *observer, MTGCardInstance * card = NULL, int _maxtargets = 1, bool other = false, bool targetMin = false) :
        TypeTargetChooser(observer, "creature", card, _maxtargets, other, targetMin)
    {
    }
    ;
    virtual bool canTarget(Targetable * target,bool withoutProtections = false);
    virtual DamageableTargetChooser * clone() const;
    virtual bool equals(TargetChooser * tc);
};

class PlayerTargetChooser: public TargetChooser
{
protected:
    Player * p; //In Case we can only target a specific player
public:
    bool withoutProtections;
    PlayerTargetChooser(GameObserver *observer, MTGCardInstance * card = NULL, int _maxtargets = 1, Player *_p = NULL);
    virtual bool canTarget(Targetable * target, bool withoutProtections = false);
    virtual PlayerTargetChooser * clone() const;
    virtual bool equals(TargetChooser * tc);
};

class DescriptorTargetChooser: public TargetZoneChooser
{
public:
    CardDescriptor * cd;
    bool withoutProtections;
    DescriptorTargetChooser(GameObserver *observer, CardDescriptor * _cd, MTGCardInstance * card = NULL, int _maxtargets = 1, bool other = false,bool targetMin = false);
    DescriptorTargetChooser(GameObserver *observer, CardDescriptor * _cd, int * _zones, int nbzones, MTGCardInstance * card = NULL, int _maxtargets = 1, bool other = false,bool targetMin = false);
    virtual bool canTarget(Targetable * target,bool withoutProtections = false);
    ~DescriptorTargetChooser();
    virtual DescriptorTargetChooser * clone() const;
    virtual bool equals(TargetChooser * tc);
};

class SpellTargetChooser: public TargetChooser
{
public:
    int color;
    bool withoutProtections;
    SpellTargetChooser(GameObserver *observer, MTGCardInstance * card = NULL, int _color = -1, int _maxtargets = 1, bool other = false, bool targetMin = false);
    virtual bool canTarget(Targetable * target, bool withoutProtections = false);
    virtual SpellTargetChooser * clone() const;
    virtual bool equals(TargetChooser * tc);
};

class SpellOrPermanentTargetChooser: public TargetZoneChooser
{
public:
    int color;
    bool withoutProtections;
    SpellOrPermanentTargetChooser(GameObserver *observer, MTGCardInstance * card = NULL, int _color = -1, int _maxtargets = 1, bool other = false, bool targetMin = false);
    virtual bool canTarget(Targetable * target, bool withoutProtections = false);
    virtual SpellOrPermanentTargetChooser * clone() const;
    virtual bool equals(TargetChooser * tc);
};

class DamageTargetChooser: public TargetChooser
{
public:
    int color;
    int state;
    bool withoutProtections;
    DamageTargetChooser(GameObserver *observer, MTGCardInstance * card = NULL, int _color = -1, int _maxtargets = 1, int state = NOT_RESOLVED);
    virtual bool canTarget(Targetable * target, bool withoutProtections = false);
    virtual DamageTargetChooser * clone() const;
    virtual bool equals(TargetChooser * tc);
};

//Should only be used for triggered abilities.
class TriggerTargetChooser: public TargetChooser
{
public:
    Targetable * target;
    int triggerTarget;
    bool withoutProtections;
    TriggerTargetChooser(GameObserver *observer, int _triggerTarget);
    virtual bool targetsZone(MTGGameZone * z);
    virtual bool canTarget(Targetable * _target, bool withoutProtections = false);
    virtual TriggerTargetChooser * clone() const;
    virtual bool equals(TargetChooser * tc);
};

class ProliferateChooser: public TypeTargetChooser
{
public:
    bool withoutProtections;
    ProliferateChooser(GameObserver *observer, int * _zones, int _nbzones, MTGCardInstance * card = NULL, int _maxtargets = 1, bool other = false, bool targetMin = false) :
    TypeTargetChooser(observer, "*",_zones, _nbzones, card, _maxtargets, other, targetMin)
    {
    }
    ;
    ProliferateChooser(GameObserver *observer, MTGCardInstance * card = NULL, int _maxtargets = 1, bool other = false,bool targetMin = false) :
        TypeTargetChooser(observer, "*", card, _maxtargets, other,targetMin)
    {
    }
    ;
    virtual bool canTarget(Targetable * target, bool withoutProtections = false);
    virtual ProliferateChooser * clone() const;
    virtual bool equals(TargetChooser * tc);
};

class BlockableChooser: public TypeTargetChooser
{
public:
    bool withoutProtections;
    BlockableChooser(GameObserver *observer, int * _zones, int _nbzones, MTGCardInstance * card = NULL, int _maxtargets = 1, bool other = false, bool targetMin = false) :
    TypeTargetChooser(observer, "creature",_zones, _nbzones, card, _maxtargets, other, targetMin)
    {
    }
    ;
    BlockableChooser(GameObserver *observer, MTGCardInstance * card = NULL, int _maxtargets = 1, bool other = false,bool targetMin = false) :
        TypeTargetChooser(observer, "creature", card, _maxtargets, other,targetMin)
    {
    }
    ;
    virtual bool canTarget(Targetable * target, bool withoutProtections = false);
    virtual BlockableChooser * clone() const;
    virtual bool equals(TargetChooser * tc);
};

class pairableChooser: public TypeTargetChooser
{
public:
    bool withoutProtections;
    pairableChooser(GameObserver *observer, int * _zones, int _nbzones, MTGCardInstance * card = NULL, int _maxtargets = 1, bool other = false, bool targetMin = false) :
    TypeTargetChooser(observer, "creature|mybattlefield",_zones, _nbzones, card, _maxtargets, other, targetMin)
    {
    }
    ;
    pairableChooser(GameObserver *observer, MTGCardInstance * card = NULL, int _maxtargets = 1, bool other = false,bool targetMin = false) :
        TypeTargetChooser(observer, "creature|mybattlefield", card, _maxtargets, other,targetMin)
    {
    }
    ;
    virtual bool canTarget(Targetable * target, bool withoutProtections = false);
    virtual pairableChooser * clone() const;
    virtual bool equals(TargetChooser * tc);
};

class myCursesChooser: public TypeTargetChooser
{
public:
    bool withoutProtections;
    myCursesChooser(GameObserver *observer, int * _zones, int _nbzones, MTGCardInstance * card = NULL, int _maxtargets = 1, bool other = false, bool targetMin = false) :
    TypeTargetChooser(observer, "*",_zones, _nbzones, card, _maxtargets, other, targetMin)
    {
    }
    ;
    myCursesChooser(GameObserver *observer, MTGCardInstance * card = NULL, int _maxtargets = 1, bool other = false,bool targetMin = false) :
        TypeTargetChooser(observer, "*", card, _maxtargets, other,targetMin)
    {
    }
    ;
    virtual bool canTarget(Targetable * target, bool withoutProtections = false);
    virtual myCursesChooser * clone() const;
    virtual bool equals(TargetChooser * tc);
};

class ParentChildChooser: public TypeTargetChooser
{
public:
    bool withoutProtections;
    int type;
    TargetChooser * deeperTargeting;
    ParentChildChooser(GameObserver *observer, int * _zones, int _nbzones, MTGCardInstance * card = NULL, int _maxtargets = 1,TargetChooser * deepTc = NULL,int type = 1, bool other = false, bool targetMin = false) :
    TypeTargetChooser(observer, "*",_zones, _nbzones, card, _maxtargets, other, targetMin),type(type),deeperTargeting(deepTc)
    {
    }
    ;
    ParentChildChooser(GameObserver *observer, MTGCardInstance * card = NULL, int _maxtargets = 1,TargetChooser * deepTc = NULL,int type = 1, bool other = false,bool targetMin = false) :
        TypeTargetChooser(observer, "*", card, _maxtargets, other,targetMin),type(type),deeperTargeting(deepTc)
    {
    }
    ;
    virtual bool canTarget(Targetable * target, bool withoutProtections = false);
    virtual ParentChildChooser * clone() const;
    virtual bool equals(TargetChooser * tc);
    ~ParentChildChooser();
};
#endif
