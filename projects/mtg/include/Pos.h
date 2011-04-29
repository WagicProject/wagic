#ifndef _POS_H_
#define _POS_H_

#include "JGE.h"

struct Pos
{
    float actX, actY, actZ, actT, actA;
    float x, y, zoom, t, alpha;
    PIXEL_TYPE mask;
    Pos(float, float, float, float, float);
    virtual void Update(float dt);
    void UpdateNow();
    virtual void Render();
    void Render(JQuad*);
};

#endif // _POS_H_
