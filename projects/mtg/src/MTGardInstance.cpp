#include "MTGCardInstance.h"

//---------------------------------------------
//Card Instance
//--------------------------------------------
MTGCardInstance::MTGCardInstance(MTGCard * card, MTGPlayerCards * _belongs_to) :
    model(card)
{
    belongs_to = _belongs_to;
}

int MTGCardInstance::isInPlay()
{
    return belongs_to->isInPlay(this);
}

JQuad * MTGCardInstance::getQuad(TexturesCache * cache)
{
    return model->getQuad(cache);
}
