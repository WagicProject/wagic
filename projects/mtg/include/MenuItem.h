#ifndef _MENU_ITEM_H
#define _MENU_ITEM_H

#include <JLBFont.h>
#include <JGui.h>
#include <hge/hgeparticle.h>
#include <string>
using namespace std;

#define SCALE_SELECTED		1.2f
#define SCALE_NORMAL		1.0f

class hgeParticleSystem;

class MenuItem: public JGuiObject
{
 private:
  bool mHasFocus;
  JLBFont *mFont;
  string mText;
  int mX;
  int mY;
  int updatedSinceLastRender;
  float lastDt;

  float mScale;
  float mTargetScale;
  JQuad * onQuad;
  JQuad * offQuad;
  hgeParticleSystem*	mParticleSys;


 public:
  MenuItem(int id, JLBFont *font, string text, int x, int y, JQuad * _off, JQuad * _on, const char * particle, JQuad * particleQuad, bool hasFocus = false);
  ~MenuItem();
  virtual void Render();
  virtual void Update(float dt);

  virtual void Entering();
  virtual bool Leaving(u32 key);
  virtual bool ButtonPressed();
};

#endif

