/* Graphical representation of a Card Instance, used in game */

#ifndef _CARD_GUI_H_
#define _CARD_GUI_H_

#include <hge/hgeparticle.h>
#include <JGui.h>
#include "Pos.h"
#include "PlayGuiObject.h"
#include "MTGCardInstance.h"

class MTGCardInstance;
class PlayGuiObject;

namespace DrawMode
{
    enum
    {
        kNormal  = 0,
        kText,
        kHidden
    };
    const int kNumDrawModes = 3;
}

struct CardGui: public PlayGuiObject
{
protected:

    /*
    ** Tries to render the Big version of a card picture, backups to text version in case of failure
    */
    static void RenderBig(MTGCard * card, const Pos& pos);

    void RenderCountersBig(const Pos& pos);
    static void AlternateRender(MTGCard * card, const Pos& pos);
    static void TinyCropRender(MTGCard * card, const Pos& pos, JQuad * quad);

public:
    static const float Width;
    static const float Height;
    static const float BigWidth;
    static const float BigHeight;

    MTGCardInstance* card;
    CardGui(MTGCardInstance* card, float x, float y);
    CardGui(MTGCardInstance* card, const Pos& ref);
    virtual void Render();
    virtual void Update(float dt);

    void DrawCard(const Pos& inPosition, int inMode = DrawMode::kNormal);
    static void DrawCard(MTGCard* inCard, const Pos& inPosition, int inMode = DrawMode::kNormal);

    static JQuadPtr AlternateThumbQuad(MTGCard * card);
    virtual ostream& toString(ostream&) const;
};

class CardView: public CardGui
{
public:

    typedef enum
    {
        nullZone, handZone, playZone
    } SelectorZone;

    const SelectorZone owner;

    MTGCardInstance* getCard(); // remove this when possible
    CardView(const SelectorZone, MTGCardInstance* card, float x, float y);
    CardView(const SelectorZone, MTGCardInstance* card, const Pos& ref);
    virtual ~CardView();

    void Render()
    {
        CardGui::Render();
    }
    
    void Render(JQuad* q)
    {
        Pos::Render(q);
    }
    
    virtual ostream& toString(ostream&) const;

    float GetCenterX();
    float GetCenterY();
};

class TransientCardView: public CardGui
{
public:
    TransientCardView(MTGCardInstance* card, float x, float y);
    TransientCardView(MTGCardInstance* card, const Pos& ref);
};

#endif
