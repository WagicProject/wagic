/* Graphical representation of a Card Instance, used in game */

#ifndef _CARD_GUI_H_
#define _CARD_GUI_H_

#include <hge/hgeparticle.h>
#include <JGui.h>
#include "Pos.h"
#include "PlayGuiObject.h"
#include "MTGCardInstance.h"
#include "CardSelector.h"

class MTGCardInstance;
class PlayGuiObject;

struct CardGui : public PlayGuiObject {
 protected:

 public:
  static const float Width;
  static const float Height;
  static const float BigWidth;
  static const float BigHeight;

  MTGCardInstance* card;
  CardGui(MTGCardInstance* card, float x, float y);
  CardGui(MTGCardInstance* card, const Pos& ref);
  virtual void Render();
  void RenderBig(const Pos&); //Tries to render the Big version of a card picture, backups to text version in case of failure
  static void RenderBig(MTGCard * card, const Pos& pos);
  void alternateRenderBig(const Pos&); //Renders Text Version of a card
  void renderCountersBig(const Pos& pos);
  virtual void Update(float dt);
  static void alternateRender(MTGCard * card, const Pos& pos);
  static JQuad * alternateThumbQuad(MTGCard * card);
  virtual ostream& toString(ostream&) const;
};

class CardView : public CardGui {
 public:
  const CardSelector::SelectorZone owner;

  MTGCardInstance* getCard(); // remove this when possible
  CardView(const CardSelector::SelectorZone, MTGCardInstance* card, float x, float y);
  CardView(const CardSelector::SelectorZone, MTGCardInstance* card, const Pos& ref);
  void Render(){CardGui::Render();};
  void Render(JQuad* q){Pos::Render(q);};
  virtual ostream& toString(ostream&) const;
};

class TransientCardView : public CardGui {
 public:
  TransientCardView(MTGCardInstance* card, float x, float y);
  TransientCardView(MTGCardInstance* card, const Pos& ref);
};

#endif
