#ifndef _GUIFRAME_H_
#define _GUIFRAME_H_

#include "GuiLayers.h"

class GuiFrame: public GuiLayer
{
protected:
    JQuad* wood;
    JQuad* gold1, *gold2, *goldGlow;
    float step;

public:
    GuiFrame();
    ~GuiFrame();
    virtual void Render();
    void Update(float dt);
};

#endif // _GUIFRAME_H_
