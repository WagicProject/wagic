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
    enum
    {
        UNSET = 0,
        OPPONENT = -1,
        CONTROLLER = 1,
        TARGET_CONTROLLER = 2,
        OWNER = 3
    };
    bool other;

    TargetChooser(MTGCardInstance * card = NULL, int _maxtargets = -1, bool other = false);

    MTGCardInstance * source;
    MTGCardInstance * targetter; //Optional, usually equals source, used for protection from...

    int maxtargets; //Set to -1 for "unlimited"
    bool validTargetsExist();
    virtual int setAllZones()
    {
        return 0;
    }
    virtual bool targetsZone(MTGGameZone * z)
    {
        return false;
    }
    ;
    int ForceTargetListReady();
    int targetsReadyCheck();
    virtual int addTarget(Targetable * target);
    virtual bool canTarget(Targetable * _target);

    //returns true if tc is equivalent to this TargetChooser
    //Two targetchoosers are equivalent if they target exactly the same cards
    virtual bool equals(TargetChooser * tc);

    virtual int full()
    {
        if (maxtargets != -1 && cursor >= maxtargets)
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
        return cursor;
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
public:
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
    TargetZoneChooser(MTGCardInstance * card = NULL, int _maxtargets = 1, bool other = false);
    TargetZoneChooser(int * _zones, int _nbzones, MTGCardInstance * card = NULL, int _maxtargets = 1, bool other = false);
    virtual bool canTarget(Targetable * _card);
    int setAllZones();
    virtual TargetZoneChooser * clone() const;
    virtual bool equals(TargetChooser * tc);
};

class CardTargetChooser: public TargetZoneChooser
{
protected:
    MTGCardInstance * validTarget;
public:
    CardTargetChooser(MTGCardInstance * card, MTGCardInstance * source, int * zones = NULL, int nbzones = 0);
    virtual bool canTarget(Targetable * target);
    virtual CardTargetChooser * clone() const;
    virtual bool equals(TargetChooser * tc);
};

class TypeTargetChooser: public TargetZoneChooser
{
public:
    int nbtypes;
    int types[10];
    TypeTargetChooser(const char * _type, MTGCardInstance * card = NULL, int _maxtargets = 1, bool other = false);
    TypeTargetChooser(const char * _type, int * _zones, int nbzones, MTGCardInstance * card = NULL, int _maxtargets = 1, bool other = false);
    void addType(int type);
    void addType(const char * type);
    virtual bool canTarget(Targetable * target);
    virtual TypeTargetChooser * clone() const;
    virtual bool equals(TargetChooser * tc);
};

class DamageableTargetChooser: public TypeTargetChooser
{
public:
    DamageableTargetChooser(int * _zones, int _nbzones, MTGCardInstance * card = NULL, int _maxtargets = 1, bool other = false) :
        TypeTargetChooser("creature",_zones, _nbzones, card, _maxtargets, other)
    {
    }
    ;
    DamageableTargetChooser(MTGCardInstance * card = NULL, int _maxtargets = 1, bool other = false) :
        TypeTargetChooser("creature", card, _maxtargets, other)
    {
    }
    ;
    virtual bool canTarget(Targetable * target);
    virtual DamageableTargetChooser * clone() const;
    virtual bool equals(TargetChooser * tc);
};

class PlayerTargetChooser: public TargetChooser
{
protected:
    Player * p; //In Case we can only target a specific player
public:
    PlayerTargetChooser(MTGCardInstance * card = NULL, int _maxtargets = 1, Player *_p = NULL);
    virtual bool canTarget(Targetable * target);
    virtual PlayerTargetChooser * clone() const;
    virtual bool equals(TargetChooser * tc);
};

class DescriptorTargetChooser: public TargetZoneChooser
{
public:
    CardDescriptor * cd;
    DescriptorTargetChooser(CardDescriptor * _cd, MTGCardInstance * card = NULL, int _maxtargets = 1, bool other = false);
    DescriptorTargetChooser(CardDescriptor * _cd, int * _zones, int nbzones, MTGCardInstance * card = NULL, int _maxtargets = 1, bool other = false);
    virtual bool canTarget(Targetable * target);
    ~DescriptorTargetChooser();
    virtual DescriptorTargetChooser * clone() const;
    virtual bool equals(TargetChooser * tc);
};

class SpellTargetChooser: public TargetChooser
{
public:
    int color;
    SpellTargetChooser(MTGCardInstance * card = NULL, int _color = -1, int _maxtargets = 1, bool other = false);
    virtual bool canTarget(Targetable * target);
    virtual SpellTargetChooser * clone() const;
    virtual bool equals(TargetChooser * tc);
};

class SpellOrPermanentTargetChooser: public TargetZoneChooser
{
public:
    int color;
    SpellOrPermanentTargetChooser(MTGCardInstance * card = NULL, int _color = -1, int _maxtargets = 1, bool other = false);
    virtual bool canTarget(Targetable * target);
    virtual SpellOrPermanentTargetChooser * clone() const;
    virtual bool equals(TargetChooser * tc);
};

class DamageTargetChooser: public TargetChooser
{
public:
    int color;
    int state;
    DamageTargetChooser(MTGCardInstance * card = NULL, int _color = -1, int _maxtargets = 1, int state = NOT_RESOLVED);
    virtual bool canTarget(Targetable * target);
    virtual DamageTargetChooser * clone() const;
    virtual bool equals(TargetChooser * tc);
};

//Should only be used for triggered abilities.
class TriggerTargetChooser: public TargetChooser
{
public:
    Targetable * target;
    int triggerTarget;
    TriggerTargetChooser(int _triggerTarget);
    virtual bool targetsZone(MTGGameZone * z);
    virtual bool canTarget(Targetable * _target);
    virtual TriggerTargetChooser * clone() const;
    virtual bool equals(TargetChooser * tc);
};

#endif
