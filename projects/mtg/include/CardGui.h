/* Graphical representation of a Card Instance, used in game */

#ifndef _CARD_GUI_H_
#define _CARD_GUI_H_

#include <JGui.h>
#include "PlayGuiObject.h"
#include "MTGCardInstance.h"
#include <hge/hgeparticle.h>

class MTGCardInstance;
class PlayGuiObject;

class CardGui: public PlayGuiObject{
 protected:
  hgeParticleSystem * mParticleSys;
  int alpha;
 public:
  MTGCardInstance * card;
  CardGui(int id, MTGCardInstance * _card, float desiredHeight, float _x=0, float _y=0, bool hasFocus = false);
  virtual void Render();
  virtual void Update(float dt);
  virtual ostream& toString(ostream& out) const;

  void RenderBig(float x=-1, float y = -1, int alternate = 0);
  static void alternateRender(MTGCard * card, JQuad ** manaIcons, float x, float y, float rotation= 0, float scale=1);
  ~CardGui();
};


#endif
