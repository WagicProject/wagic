#ifndef _GUIPHASEBAR_H_
#define _GUIPHASEBAR_H_

#include "GuiLayers.h"
#include "PhaseRing.h"
#include "WEvent.h"

class GuiPhaseBar: public GuiLayer, public PlayGuiObject
{
protected:
    Phase* phase;
    float angle;
    float zoomFactor;

public:
    GuiPhaseBar(GameObserver* observer);
    ~GuiPhaseBar();
    void Update(float dt);
    virtual void Render();
    virtual int receiveEventMinus(WEvent * e);
    virtual ostream& toString(ostream& out) const;
    virtual void Entering();
    virtual bool Leaving(JButton key);
};

#endif // _GUIPHASEBAR_H_
