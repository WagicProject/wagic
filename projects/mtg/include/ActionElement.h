/*
*  Wagic, The Homebrew ?! is licensed under the BSD license
*  See LICENSE in the Folder's root
*  http://wololo.net/wagic/
*/

#ifndef _ACTIONELEMENT_H_
#define _ACTIONELEMENT_H_
#include <JGui.h>

#define INACTIVE 0
#define ACTION_REQUESTED 1
#define ACTIVE 2


class MTGCardInstance;
class Targetable;
class TargetChooser;

class ActionElement: public JGuiObject{
protected:
	int activeState;



public:
	TargetChooser * tc;
	int currentPhase;
	int newPhase;
		int modal;
		int waitingForAnswer;
  void RenderMessageBackground(float y0, int height);
	int getActivity();
	virtual void Update(float dt){};
	virtual void Render(){};
	virtual int testDestroy(){return 0;};
	virtual int destroy(){return 0;};
	virtual void CheckUserInput(float dt){};
	ActionElement(int id);
	virtual int isReactingToTargetClick(Targetable * card);
	virtual int reactToTargetClick(Targetable * card);
	virtual int isReactingToClick(MTGCardInstance * card){return 0;};
	virtual int reactToClick(MTGCardInstance * card){return 0;};
	virtual const char * getMenuText(){return "Ability";};
};


#endif
