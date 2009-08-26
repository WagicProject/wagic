/*
  A class for very simple menus structure
*/
#ifndef _SIMPLEMENU_H_
#define _SIMPLEMENU_H_

#include <string>
#include <JGui.h>
#include <JLBFont.h>
#include "hge/hgeparticle.h"

class SimpleMenu:public JGuiController{
 private:
  // See SimpleMenu.cpp for values
  static const unsigned SIDE_SIZE;
  static const unsigned VMARGIN;
  static const unsigned HMARGIN;
  static const signed LINE_HEIGHT;

 private:
  int mHeight, mWidth, mX, mY;
  JLBFont* mFont;
  std::string title;
  int displaytitle;
  int maxItems,startId;
  float selectionT, selectionY;
  float timeOpen;
  static unsigned int refCount;

  static JQuad *spadeR, *spadeL, *jewel, *side;
  static JTexture *spadeRTex, *spadeLTex, *jewelTex, *sideTex;
  static JLBFont* titleFont;
  static hgeParticleSystem* stars;
  // This works only because of no multithreading
  static PIXEL_TYPE jewelGraphics[9];

  inline void MogrifyJewel();
  void drawHorzPole(int x, int y, int width);
  void drawVertPole(int x, int y, int height);

 public:
  SimpleMenu(int id, JGuiListener* listener, JLBFont* font, int x, int y, const char * _title = "", int _maxItems = 7);
  void Render();
  void Update(float dt);
  void Add(int id, const char * Text,string desc = "");
  void Close();

  float selectionTargetY;
  bool closed;
  static void destroy();
};


#endif
