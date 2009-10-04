#include "string.h"
#include <vector>
#include <hge/hgeparticle.h>
#include "config.h"
#include "JGE.h"
#include "MTGDefinitions.h"
#include "GameApp.h"
#include "GuiLayers.h"

class ManaIcon : public Pos
{
  hgeParticleSystem* particleSys;
  JQuad* icon;

  float zoomP1, zoomP2, zoomP3, zoomP4, zoomP5, zoomP6;
  float xP1, xP2, xP3;
  float yP1, yP2, yP3;
  float tP1;
  float f;
  float destx,desty;
 public:
  enum { ALIVE, WITHERING, DROPPING, DEAD } mode;
  int color;
  void Render();
  void Update(float dt, float shift);
  void Wither();
  void Drop();
  ManaIcon(int color, float x, float y,float destx, float desty);
  ~ManaIcon();
};

class GuiMana : public GuiLayer
{
 protected:
  vector<ManaIcon*> manas;
  float x, y;
  Player * owner;
  void RenderStatic();
 public:
  GuiMana(float x, float y, Player *p);
  ~GuiMana();
  virtual void Render();
  virtual void Update(float dt);
  virtual int receiveEventPlus(WEvent * e);
  virtual int receiveEventMinus(WEvent * e);
};
