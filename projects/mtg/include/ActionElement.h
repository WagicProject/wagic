/*
 *  Wagic, The Homebrew ?! is licensed under the BSD license
 *  See LICENSE in the Folder's root
 *  http://wololo.net/wagic/
 */

#ifndef _ACTIONELEMENT_H_
#define _ACTIONELEMENT_H_
#include <JGui.h>
#include "MTGDefinitions.h"

#define INACTIVE 0
#define ACTION_REQUESTED 1
#define ACTIVE 2

class MTGCardInstance;
class ManaCost;
class Targetable;
class TargetChooser;
class WEvent;

class ActionElement: public JGuiObject
{
protected:
    int activeState;
    TargetChooser * tc;
public:
    GamePhase currentPhase;
    GamePhase newPhase;
    int modal;
    int waitingForAnswer;
    int getActivity();
    virtual void Update(float){};
    virtual void Render(){};
    virtual int testDestroy()
    {
        return 0;
    }
    ;
    virtual int destroy()
    {
        return 0;
    }
    ;
    virtual bool CheckUserInput(JButton)
    {
        return false;
    }
    ;
    ActionElement(int id);
    ActionElement(const ActionElement& copyFromMe);
    TargetChooser * getActionTc(){return tc;}
    virtual void setActionTC(TargetChooser * newTc = NULL){this->tc = newTc;}
    virtual ~ActionElement();
    virtual int isReactingToTargetClick(Targetable * card);
    virtual int reactToTargetClick(Targetable * card);
    virtual int reactToChoiceClick(Targetable *,int,int)
    {
        return 0;
    }
    virtual int isReactingToClick(MTGCardInstance *, ManaCost * = NULL)
    {
        return 0;
    }
    ;
    virtual int stillInUse(MTGCardInstance *)
    {
        return 0;
    }
    ;
    virtual int receiveEvent(WEvent *)
    {
        return 0;
    }
    ;
    virtual int reactToClick(MTGCardInstance *)
    {
        return 0;
    }
    ;
    virtual const char * getMenuText()
    {
        return "Ability";
    }
    ;
    virtual ActionElement * clone() const = 0;

};

#endif
