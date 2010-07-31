#ifndef _SHOP_ITEM_H
#define _SHOP_ITEM_H

#include <JGui.h>
#include "../include/WFont.h"
#include "SimpleMenu.h"
#include "MTGDeck.h"
#include "../include/PriceList.h"
#include "../include/PlayerData.h"
#include "../include/CardDisplay.h"
#include "../include/DeckDataWrapper.h"

#include <string>
using std::string;

class hgeDistortionMesh;

#define SHOP_BOOSTERS 3

class ShopItem:public JGuiObject{
 private:
  friend class ShopItems;
  bool mHasFocus;
  bool mRelease;
  WFont *mFont;
  string mText;
  float xy[8];
  JQuad * quad;
  JQuad * thumb;
  float mScale;
  float mTargetScale;
  hgeDistortionMesh* mesh;

  void updateThumb();

 public:
  int nameCount;
  int quantity;
  MTGCard * card;
  int price;
  ShopItem(int id, WFont * font, int _cardid, float _xy[], bool hasFocus, MTGAllCards * collection, int _price, DeckDataWrapper * ddw);
  ShopItem(int id, WFont * font, char* text, JQuad * _quad, JQuad * _thumb,float _xy[], bool hasFocus, int _price);
  ~ShopItem();
  int updateCount(DeckDataWrapper * ddw);

  virtual void Render();
  virtual void Update(float dt);

  virtual void Entering();
  virtual bool Leaving(JButton key);
  virtual bool ButtonPressed();

  const char * getText();
  virtual ostream& toString(ostream& out) const;
};

class ShopItems:public JGuiController,public JGuiListener{
 private:
  PlayerData * playerdata;
  PriceList * pricelist;
  int  mX, mY, mHeight;
  WFont* mFont;
  JTexture * mBgAATex;
  JQuad * mBgAA;
  MTGAllCards * collection;
  SimpleMenu * dialog;
  int showPriceDialog;
  int setIds[SHOP_BOOSTERS];
  MTGCardInstance * displayCards[100];
  CardDisplay * display;
  void safeDeleteDisplay();
    DeckDataWrapper * myCollection;
    
  int lightAlpha;
  int alphaChange;

 public:
  bool showCardList;
  ShopItems(int id, JGuiListener* listener, WFont* font, int x, int y, MTGAllCards * _collection, int _setIds[]);
  ~ShopItems();
  void Render();
  virtual void Update(float dt);
  void Add(int cardid);
  void Add(char * text, JQuad * quad, JQuad * thumb,int _price);
  void pricedialog(int id, int mode=1);
  virtual void ButtonPressed(int controllerId, int controlId);
  bool CheckUserInput(JButton key);
  void savePriceList();
  void saveAll();
  static float _x1[],_y1[],_x2[],_y2[],_x3[],_y3[],_x4[],_y4[];
};

#endif
