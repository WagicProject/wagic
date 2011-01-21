#ifndef _GUIBACKGROUND_H_
#define _GUIBACKGROUND_H_

#include "GuiLayers.h"
#include "WEvent.h"

class GuiBackground: public GuiLayer
{
protected:
    JQuad* quad;

public:
    GuiBackground();
    ~GuiBackground();
    virtual void Render();
};

#endif // _GUIBACKGROUND_H_
