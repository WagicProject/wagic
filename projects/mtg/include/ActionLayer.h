/*
 *  Wagic, The Homebrew ?! is licensed under the BSD license
 *  See LICENSE in the Folder's root
 *  http://wololo.net/wagic/
 */

#ifndef _ACTIONLAYER_H_
#define _ACTIONLAYER_H_

#include "GuiLayers.h"
#include "ActionElement.h"
#include "SimpleMenu.h"
#include "MTGAbility.h"

#include <set>

class GuiLayer;
class Targetable;
class WEvent;

class ActionLayer: public GuiLayer, public JGuiListener
{
public:
    vector<ActionElement *> garbage;
    Targetable * menuObject;
    SimpleMenu * abilitiesMenu;
    MTGCardInstance * currentActionCard;
    int stuffHappened;
    virtual void Render();
    virtual void Update(float dt);
    bool CheckUserInput(JButton key);
    ActionLayer();
    ~ActionLayer();
    int cancelCurrentAction();
    ActionElement * isWaitingForAnswer();
    int isReactingToTargetClick(Targetable * card);
    int receiveEventPlus(WEvent * event);
    int reactToTargetClick(Targetable * card);
    int isReactingToClick(MTGCardInstance * card);
    int reactToClick(MTGCardInstance * card);
    int reactToClick(ActionElement * ability, MTGCardInstance * card);
    int reactToTargetClick(ActionElement * ability, Targetable * card);
    int stillInUse(MTGCardInstance * card);
    void setMenuObject(Targetable * object, bool must = false);
    void setCustomMenuObject(Targetable * object, bool must = false,vector<MTGAbility*>abilities = vector<MTGAbility*>());
    void ButtonPressed(int controllerid, int controlid);
    void doMultipleChoice(int choice = -1);
    void ButtonPressedOnMultipleChoice(int choice = -1);
    void doReactTo(int menuIndex);
    TargetChooser * getCurrentTargetChooser();
    void setCurrentWaitingAction(ActionElement * ae);
    MTGAbility * getAbility(int type);
    int checkCantCancel(){return cantCancel;};
    
    //Removes from game but does not move the element to garbage. The caller must take care of deleting the element.
    int removeFromGame(ActionElement * e);
    
    bool moveToGarbage(ActionElement * e);
    
    void cleanGarbage();

protected:
    ActionElement * currentWaitingAction;
    int cantCancel;
    std::set<ActionElement*> mReactions;
};

#endif
