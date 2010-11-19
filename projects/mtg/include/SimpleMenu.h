/*
  A class for very simple menus structure
*/
#ifndef _SIMPLEMENU_H_
#define _SIMPLEMENU_H_

#include <string>
#include <JGui.h>
#include "WFont.h"
#include "hge/hgeparticle.h"

class SimpleMenu:public JGuiController{
 private:
  float mHeight, mWidth, mX, mY;
  int fontId;
  std::string title;
  int displaytitle;
  int maxItems,startId;
  float selectionT, selectionY;
  float timeOpen;
  bool mClosed;

  static JQuad *spadeR, *spadeL, *jewel, *side;
  static JTexture *spadeRTex, *spadeLTex, *jewelTex, *sideTex;
  static WFont* titleFont;
  static hgeParticleSystem* stars;

  inline void MogrifyJewel();
  void drawHorzPole(float x, float y, float width);
  void drawVertPole(float x, float y, float height);

 public:
  bool autoTranslate;
  SimpleMenu(int id, JGuiListener* listener, int fontId, float x, float y, const char * _title = "", int _maxItems = 7);
  void Render();
  void Update(float dt);
  void Add(int id, const char * Text,string desc = "", bool forceFocus = false);
  void Close();

  float selectionTargetY;
  bool isClosed() { return mClosed; }
  static void destroy();
};


#endif
