/*
 *  Wagic, The Homebrew ?! is licensed under the BSD license
 *  See LICENSE in the Folder's root
 *  http://wololo.net/wagic/
 */

#ifndef _ACTIONELEMENT_H_
#define _ACTIONELEMENT_H_
#include <JGui.h>
#include "MTGDefinitions.h"

class MTGCardInstance;
class ManaCost;
class Targetable;
class TargetChooser;
class WEvent;

class ActionElement: public JGuiObject
{
protected:
    enum Activity{
        Inactive,
        ActionRequested,
        Active
    };

    Activity activity;
    TargetChooser * tc;
public:
    GamePhase currentPhase;
    GamePhase newPhase;
    int modal;
    int waitingForAnswer;
    virtual void Update(float){}
    virtual void Render(){}
    virtual int testDestroy()
    {
        return 0;
    }
    virtual int destroy()
    {
        return 0;
    }
    virtual bool CheckUserInput(JButton)
    {
        return false;
    }
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
    virtual int stillInUse(MTGCardInstance *)
    {
        return 0;
    }
    virtual int receiveEvent(WEvent *)
    {
        return 0;
    }
    virtual int reactToClick(MTGCardInstance *)
    {
        return 0;
    }
    virtual const char * getMenuText()
    {
        return "Ability";
    }
    virtual ActionElement * clone() const = 0;

};

#endif
