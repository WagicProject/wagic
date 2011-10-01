#ifndef _GUIFRAME_H_
#define _GUIFRAME_H_

#include "GuiLayers.h"

class GuiFrame : public GuiLayer
{
protected:
    JQuadPtr wood;
    JQuadPtr gold1, gold2, goldGlow;
    float step;

public:
    GuiFrame(GameObserver* observer);
    ~GuiFrame();
    virtual void Render();
    void Update(float dt);
};

#endif // _GUIFRAME_H_
