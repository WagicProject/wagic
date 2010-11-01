/*
  A class for very simple menus structure
*/
#ifndef _DeckMenu_H_
#define _DeckMenu_H_

#include <string>
#include <JGui.h>
#include "WFont.h"
#include "hge/hgeparticle.h"
#include "DeckMetaData.h"


class DeckMenu:public JGuiController{
 private:
  int mHeight, mWidth, mX, mY;
  int titleX, titleY, titleWidth;
  int descX, descY, descHeight, descWidth;
  int statsX, statsY, statsHeight, statsWidth;

  int fontId;
  std::string title;
  int displaytitle;
  int maxItems, startId;
  float selectionT, selectionY;
  float timeOpen;
  static unsigned int refCount;

  JQuad  *background;
  JTexture *backgroundTexture;
  static WFont* titleFont;
  static hgeParticleSystem* stars;
  // This works only because of no multithreading
  static PIXEL_TYPE jewelGraphics[9];

  inline void MogrifyJewel();

 public:
  
  bool autoTranslate;
  DeckMenu(int id, JGuiListener* listener, int fontId, const char * _title = "");
  void Render();
  void Update(float dt);
  void Add(int id, const char * Text, string desc = "", bool forceFocus = false, DeckMetaData *deckMetaData = NULL);
  void Close();

  float selectionTargetY;
  bool closed;
  static void destroy();
};


#endif
