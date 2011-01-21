#ifndef _MTGGAMEPHASE_H_
#define _MTGGAMEPHASE_H_

#include "ActionElement.h"
#include "GameObserver.h"

#include <JGui.h>
#include "WFont.h"

class MTGGamePhase: public ActionElement
{
protected:
    float animation;
    int currentState;
    WFont * mFont;
public:
    MTGGamePhase(int id);
    virtual void Update(float dt);
    bool CheckUserInput(JButton key);
    virtual MTGGamePhase * clone() const;
    virtual ostream& toString(ostream& out) const;
};

#endif
