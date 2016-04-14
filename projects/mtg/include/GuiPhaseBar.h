#ifndef _GUIPHASEBAR_H_
#define _GUIPHASEBAR_H_

#include "GuiLayers.h"
#include "PhaseRing.h"
#include "WEvent.h"
#include "PlayGuiObject.h"

#include "Easing.h"

class GuiPhaseBar: public GuiLayer, public PlayGuiObject
{
private:
    static const float zoom_big;
    static const float zoom_small;
    static const float step;

    int displayedPhaseId;
    float angle;
    float zoomFactor;
    OutQuadEasing angleEasing;
    InOutQuadEasing zoomFactorEasing;
    DuelLayers* mpDuelLayers;

    void DrawGlyph(JQuad *inQuad, int phaseId, float x, float y, float scale, float z);
public:
    GuiPhaseBar(DuelLayers* duelLayers);
    ~GuiPhaseBar();
    void Update(float dt);
    virtual void Render();
    virtual int receiveEventMinus(WEvent * e);
    virtual ostream& toString(ostream& out) const;
    virtual void Entering();
    virtual bool Leaving(JButton key);
};

#endif // _GUIPHASEBAR_H_
