#ifndef _SHOP_ITEM_H
#define _SHOP_ITEM_H

#include <JGui.h>
#include <JLBFont.h>
#include "SimpleMenu.h"
#include "MTGDeck.h"
#include "../include/PriceList.h"
#include "../include/PlayerData.h"
#include "../include/CardDisplay.h"
#include "../include/DeckDataWrapper.h"

#include <string>
using std::string;

#define SHOP_BOOSTERS 3

class ShopItem:public JGuiObject{
 private:
  bool mHasFocus;
  JLBFont *mFont;
  string mText;
  int mX;
  int mY;
  JQuad * quad;
  JQuad * thumb;
  float mScale;
  float mTargetScale;


 public:
  int nameCount;
  int quantity;
  MTGCard * card;
  int price;
  ShopItem(int id, JLBFont * font, int _cardid, int x, int y, bool hasFocus, MTGAllCards * collection, int _price, DeckDataWrapper * ddw);
  ShopItem(int id, JLBFont * font, char* text, JQuad * _quad, JQuad * _thumb,int x, int y, bool hasFocus, int _price);
  ~ShopItem();
  int updateCount(DeckDataWrapper * ddw);

  virtual void Render();
  virtual void Update(float dt);

  virtual void Entering();
  virtual bool Leaving(u32 key);
  virtual bool ButtonPressed();

  const char * getText();
  virtual ostream& toString(ostream& out) const;
};

class ShopItems:public JGuiController,public JGuiListener{
 private:
  PlayerData * playerdata;
  PriceList * pricelist;
  int  mX, mY, mHeight;
  JLBFont* mFont;
  MTGAllCards * collection;
  SimpleMenu * dialog;
  int showPriceDialog;
  int setIds[SHOP_BOOSTERS];
  MTGCardInstance * displayCards[100];
  CardDisplay * display;
  void safeDeleteDisplay();
    DeckDataWrapper * myCollection;
 public:
  ShopItems(int id, JGuiListener* listener, JLBFont* font, int x, int y, MTGAllCards * _collection, int _setIds[]);
  ~ShopItems();
  void Render();
  virtual void Update(float dt);
  void Add(int cardid);
  void Add(char * text, JQuad * quad, JQuad * thumb,int _price);
  void pricedialog(int id, int mode=1);
  virtual void ButtonPressed(int controllerId, int controlId);
  void savePriceList();
  void saveAll();
};

#endif
