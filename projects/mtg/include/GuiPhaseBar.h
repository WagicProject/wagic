#ifndef _GUIPHASEBAR_H_
#define _GUIPHASEBAR_H_

#include "GuiLayers.h"
#include "PhaseRing.h"
#include "WEvent.h"

class GuiPhaseBar: public GuiLayer
{
protected:
    Phase* phase;
    float angle;
    float zoomTarget;
    float zoomFactor;
    static GuiPhaseBar*instance;

public:
    static GuiPhaseBar* GetInstance();

public:
    GuiPhaseBar();
    ~GuiPhaseBar();
    void Update(float dt);
    void Zoom(float);
    virtual void Render();
    virtual int receiveEventMinus(WEvent * e);
};

#endif // _GUIPHASEBAR_H_
