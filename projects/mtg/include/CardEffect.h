#ifndef _CARDEFFECT_H_
#define _CARDEFFECT_H_

#include <JGE.h>
#include "Effects.h"

struct CardGui;

class CardEffect: public Effect
{
public:
    CardEffect(CardGui* target);
    ~CardEffect();
private:
    CardGui* target;

public:
    virtual void Render();
};

#endif // _CARDEFFECT_H_
