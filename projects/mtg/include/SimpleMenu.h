/*
  A class for very simple menus structure
*/
#ifndef _SIMPLEMENU_H_
#define _SIMPLEMENU_H_

#include <string>
#include <JGui.h>
#include "../include/WFont.h"
#include "hge/hgeparticle.h"

class SimpleMenu:public JGuiController{
 private:
  int mHeight, mWidth, mX, mY;
  int fontId;
  std::string title;
  int displaytitle;
  int maxItems,startId;
  float selectionT, selectionY;
  float timeOpen;
  static unsigned int refCount;

  static JQuad *spadeR, *spadeL, *jewel, *side;
  static JTexture *spadeRTex, *spadeLTex, *jewelTex, *sideTex;
  static WFont* titleFont;
  static hgeParticleSystem* stars;
  // This works only because of no multithreading
  static PIXEL_TYPE jewelGraphics[9];

  inline void MogrifyJewel();
  void drawHorzPole(int x, int y, int width);
  void drawVertPole(int x, int y, int height);

 public:
  bool autoTranslate;
  SimpleMenu(int id, JGuiListener* listener, int fontId, int x, int y, const char * _title = "", int _maxItems = 7);
  void Render();
  void Update(float dt);
  void Add(int id, const char * Text,string desc = "", bool forceFocus = false);
  void Close();

  float selectionTargetY;
  bool closed;
  static void destroy();
};


#endif
