/* Graphical representation of a Card Instance, used in game */

#ifndef _CARD_GUI_H_
#define _CARD_GUI_H_

#include <JGui.h>
#include "Pos.h"
#include "PlayGuiObject.h"
#include "MTGCardInstance.h"
#include <hge/hgeparticle.h>

class MTGCardInstance;
class PlayGuiObject;

struct CardGui : public PlayGuiObject {
 protected:
  JQuad* quad;

 public:
  static const float Width;
  static const float Height;
  static const float BigWidth;
  static const float BigHeight;

  MTGCardInstance* card;
  CardGui(MTGCardInstance* card, float x, float y);
  CardGui(MTGCardInstance* card, const Pos& ref);
  virtual void Render() = 0;
  void RenderBig(const Pos&);
  virtual void Update(float dt) = 0;

  virtual ostream& toString(ostream&) const;
};

class CardView : public CardGui {
 public:

  MTGCardInstance* getCard(); // remove this when possible
  CardView(MTGCardInstance* card, float x, float y);
  CardView(MTGCardInstance* card, const Pos& ref);
  virtual void Render();
  void Render(JQuad* q){Pos::Render(q);};
  void RenderSelected();
  virtual void Update(float dt);
  virtual ostream& toString(ostream&) const;
};

class TransientCardView : public CardView {
 public:
  TransientCardView(MTGCardInstance* card, float x, float y);
  TransientCardView(MTGCardInstance* card, const Pos& ref);
  virtual void Render();
};


/*
class CardGui: public PlayGuiObject{
 protected:
  hgeParticleSystem * mParticleSys;
  int alpha;
  float actX, actY;
 public:
  MTGCardInstance * card;
  CardGui(int id, MTGCardInstance * _card, float desiredHeight, float _x=0, float _y=0, bool hasFocus = false);
  virtual void Render();
  virtual void Update(float dt);
  virtual ostream& toString(ostream& out) const;

  float Height();
  float Width();

  void RenderBig(float x=-1, float y = -1, int alternate = 0);
  static void alternateRender(MTGCard * card, JQuad ** manaIcons, float x, float y, float rotation= 0, float scale=1);
  ~CardGui();
};
*/

#endif
