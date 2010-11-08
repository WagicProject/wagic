/*
  A class for very simple menus structure
*/
#ifndef _DeckMenu_H_
#define _DeckMenu_H_

#include <string>
#include "WFont.h"
#include "hge/hgeparticle.h"
#include "DeckMetaData.h"
#include "TextScroller.h"

class DeckMenu:public JGuiController{
 protected:

  float mHeight, mWidth, mX, mY;
  float titleX, titleY, titleWidth;
  float descX, descY, descHeight, descWidth;
  float statsX, statsY, statsHeight, statsWidth;
  float avatarX, avatarY;
  float starsOffsetX;

  bool menuInitialized;
  string backgroundName;

  int fontId;
  string title;
  string displayTitle;
  WFont * mFont;
  float mTitleFontScale;

  int maxItems, startId;

  float selectionT, selectionY;
  float timeOpen;

  static hgeParticleSystem* stars;

  void initMenuItems();
  string getDescription();
  string getMetaInformation();

 public:
   TextScroller * scroller;
   bool autoTranslate;
   DeckMenu(int id, JGuiListener* listener, int fontId, const string _title = "");
  ~DeckMenu();
  
  void Render();
  void Update(float dt);
  void Add(int id, const char * Text, string desc = "", bool forceFocus = false, DeckMetaData *deckMetaData = NULL);
  void Close();
  void updateScroller();
  void RenderBackground();

  float selectionTargetY;
  bool closed;
  static void destroy();
};


#endif
