#ifndef _OPTION_ITEM_H_
#define _OPTION_ITEM_H_

#include <JGui.h>
#include <vector>
#include <string>
#include "../include/GameApp.h"
#include "../include/GameOptions.h"

using std::string;

#define MAX_OPTION_TABS 5
#define MAX_ONSCREEN_OPTIONS 8
#define OPTION_CENTER 4
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

class WGuiColor{
public:
  enum {
    SCROLLBAR,
    SCROLLBUTTON,
    //Foregrounds only after this
    TEXT,
    TEXT_HEADER,
    TEXT_FAIL,
    TEXT_TAB,
    TEXT_BODY,
    //Backgrounds only after this
    BACK,
    BACK_HEADER,
    BACK_FAIL,
    BACK_TAB,
  };
};

//Complete item interface
class WGuiBase{
public:
  WGuiBase() {};
  virtual ~WGuiBase() {};

  virtual bool Selectable() {return true;};
  virtual bool isModal() {return false;};
  virtual bool Visible() {return true;};
  
  virtual bool Changed() {return false;};
  virtual void confirmChange(bool confirmed) {};
  virtual PIXEL_TYPE getColor(int type);
  
  virtual void Entering(u32 key)=0;  
  virtual bool Leaving(u32 key)=0;

  virtual void Update(float dt)=0;
  virtual void updateValue(){};
  virtual void Render()=0;
  virtual void setData()=0;
  virtual void ButtonPressed(int controllerId, int controlId){};
  virtual void Reload(){};
  virtual void Overlay(){};
  virtual void Underlay(){};

  virtual bool hasFocus()=0;
  virtual void setFocus(bool bFocus)=0;
  virtual float getX()=0;
  virtual float getY()=0;
  virtual float getWidth()=0;
  virtual float getHeight()=0;
  virtual int getId() {return INVALID_ID;};
  virtual string getDisplay(){return "";};
  
  virtual void setModal(bool val){};
  virtual void setDisplay(string s){};
  virtual void setX(float _x){};
  virtual void setY(float _y){};
  virtual void setWidth(float _w){};
  virtual void setHeight(float _h){};
  virtual void setId(int _id){};
  virtual void setHidden(bool bHidden) {};
  virtual void setVisible(bool bVisisble) {};

  virtual void renderBack(WGuiBase * it);
  virtual void subBack(WGuiBase * item) {};
};

//This is our base class for concrete items. 
class WGuiItem: public WGuiBase{
public:
  virtual void Entering(u32 key);  
  virtual bool Leaving(u32 key);
  virtual void Update(float dt);
  virtual void Render();

  WGuiItem(string _display, u8 _mF = 0);
  virtual ~WGuiItem() {};

  string _(string input); //Override global with our flag checker.
  
  virtual void setData(){};

  virtual bool hasFocus() {return mFocus;};
  virtual void setFocus(bool bFocus) {mFocus = bFocus;};

  virtual string getDisplay(){return displayValue;};
  virtual void setDisplay(string s){displayValue=s;};  

  virtual int getId() {return INVALID_ID;};
  virtual float getX() {return x;};
  virtual float getY() {return y;};
  virtual float getWidth() {return width;};
  virtual float getHeight() {return height;};  
  virtual void setId(int _id){};
  virtual void setX(float _x){x = _x;};
  virtual void setY(float _y){y = _y;};
  virtual void setWidth(float _w){width = _w;};
  virtual void setHeight(float _h){height = _h;};
  
  enum {
    NO_TRANSLATE = (1<<1),
  };

  u8 mFlags;

protected:
  bool mFocus;
  float x, y;
  float width, height;
  string displayValue;
};

class OptionItem: public WGuiItem{
public:
  OptionItem( int _id,  string _displayValue);
  virtual ~OptionItem() {};

  //Accessors
  virtual int getId() {return id;};
  virtual void setId(int _id){id = _id;};

protected:
  int id;
};

class WDataSource{
public:
  WDataSource() {};
  virtual JQuad * getImage() {return NULL;};
  virtual MTGCard * getCard() {return NULL;};
  virtual bool thisCard(int mtgid) {return false;};
  virtual int getControlID() {return -1;}; //TODO FIXME: Need a "not a valid button" define.
  virtual int getPos() {return -1;};
  virtual bool setPos(int pos) {return false;};
  virtual bool next() {return false;};
  virtual bool prev() {return false;};
  virtual void Update(float dt) {};
};

class WSrcImage: public WDataSource{
public:
  virtual JQuad * getImage();
  WSrcImage(string s);

protected:
  string filename;
};

class WSrcMTGSet: public WDataSource{
public:
  WSrcMTGSet(int setid, float mDelay=0.2);
  
  virtual JQuad * getImage();
  virtual MTGCard * getCard();

  virtual bool thisCard(int mtgid);  
  virtual bool next();
  virtual bool prev();
  virtual int getPos() {return currentCard;};
  virtual bool setPos(int pos);
  virtual void Update(float dt);

protected:
  vector<MTGCard*> cards;
  int currentCard;
  float mDelay;
  float mLastInput;
};


struct WCardSort{
public:
  virtual bool operator()(const MTGCard*l, const MTGCard*r) = 0;
};

struct WCSortCollector: public WCardSort{
  bool operator()(const MTGCard*l, const MTGCard*r);
};

struct WCSortAlpha: public WCardSort{
  bool operator()(const MTGCard*l, const MTGCard*r);
};

class WGuiImage: public WGuiItem{
public:
  WGuiImage(WDataSource * wds, float _w = 0, float _h = 0, int _margin = 0);
  virtual bool Selectable() {return false;};
  virtual void Render();
  virtual float getHeight();
  virtual void imageScale(float _w, float _h);
protected:
  int margin;
  float imgW, imgH;
  WDataSource * source;
};

class WGuiCardImage: public WGuiImage{
public:
  WGuiCardImage(WDataSource * wds, int _offset=0);
  virtual void Render();
protected:
  int offset;
};

//This is our base class for decorators. It wraps everything about WGuiBase.
class WGuiDeco: public WGuiBase{
public:
  WGuiDeco(WGuiBase* _it) {it = _it;};
  virtual ~WGuiDeco() {SAFE_DELETE(it);};
  
  virtual bool Selectable() {return it->Selectable();};
  virtual bool Visible() {return it->Visible();};
  virtual bool Changed() {return it->Changed();};
  virtual void confirmChange(bool confirmed) {it->confirmChange(confirmed);};
 
  virtual void Entering(u32 key)       {it->Entering(key);};  
  virtual bool Leaving(u32 key)        {return it->Leaving(key);};
  virtual void Update(float dt) {it->Update(dt);};
  virtual void updateValue()    {it->updateValue();};
  virtual void Reload()         {it->Reload();};
  virtual void Overlay()        {it->Overlay();};
  virtual void Underlay()        {it->Underlay();};
  virtual void Render()         {it->Render();};
  virtual void setData()        {it->setData();};

  virtual void ButtonPressed(int controllerId, int controlId) {it->ButtonPressed(controllerId, controlId);};

  virtual bool hasFocus()             {return it->hasFocus();};
  virtual string getDisplay()         {return it->getDisplay();};
  virtual int getId()       {return it->getId();};
  virtual float getX()      {return it->getX();};
  virtual float getY()      {return it->getY();};
  virtual float getWidth()  {return it->getWidth();};
  virtual float getHeight() {return it->getHeight();};
  virtual PIXEL_TYPE getColor(int type) {return it->getColor(type);};
  WGuiBase * getDecorated() {return it;};
  
  virtual void setFocus(bool bFocus)  {it->setFocus(bFocus);};
  virtual void setDisplay(string s)   {it->setDisplay(s);};
  virtual void setId(int _id)      {it->setId(_id);};
  virtual void setX(float _x)      {it->setX(_x);};
  virtual void setY(float _y)      {it->setY(_y);};
  virtual void setWidth(float _w)  {it->setWidth(_w);};
  virtual void setHeight(float _h) {it->setHeight(_h);};
  virtual void setHidden(bool bHidden) {it->setHidden(bHidden);};
  virtual void setVisible(bool bVisisble) {it->setVisible(bVisisble);};
protected:
  WGuiBase * it;
};

class WGuiAward: public WGuiItem{
public:
  WGuiAward(int _id, string name, string _text, string _details="");
  virtual ~WGuiAward();
  virtual void Render();
  virtual bool Selectable() {return Visible();};
  virtual bool Visible();  
  virtual int getId() {return id;};
  virtual void Underlay();
  virtual void Overlay();

protected:
  string details;
  int id;
  string text;
};

class WGuiSplit: public WGuiItem{
public:
  WGuiSplit(WGuiBase* _left,WGuiBase* _right);
  virtual ~WGuiSplit();

  virtual void Reload();
  virtual void Overlay();
  virtual void Underlay();
  virtual void setData();
  virtual bool isModal();
  virtual void setModal(bool val);
  virtual void Render();
  virtual void Update(float dt);
  virtual void setX(float _x);
  virtual void setY(float _y);
  virtual void setWidth(float _w);
  virtual void setHeight(float _h);
  virtual float getHeight();
  virtual void ButtonPressed(int controllerId, int controlId);
  virtual void confirmChange(bool confirmed);

  virtual void Entering(u32 key);  
  virtual bool Leaving(u32 key);

  bool bRight;
  float percentRight;
  WGuiBase* right;
  WGuiBase* left;
};

class WDecoConfirm: public WGuiDeco{
public:
  WDecoConfirm(JGuiListener * _listener, WGuiBase * it);
  virtual ~WDecoConfirm();

  virtual bool isModal();
  virtual void setData();
  virtual void setModal(bool val);
  virtual void Entering(u32 key);
  virtual bool Leaving(u32 key);
  virtual void Update(float dt);
  virtual void Overlay();
  virtual void ButtonPressed(int controllerId, int controlId);

  string confirm;
  string cancel;
protected:
  enum {
    OP_UNCONFIRMED,
    OP_CONFIRMING,
    OP_CONFIRMED,
  } mState;
  SimpleMenu * confirmMenu;
  JGuiListener * listener;
  bool bModal;
};

class WDecoEnum : public WGuiDeco {
 public:
  WDecoEnum(WGuiBase * _it,EnumDefinition *_edef = NULL);
  virtual void Render();
  string lookupVal(int value);
 protected:
  EnumDefinition * edef;
};

class WDecoCheat: public WGuiDeco {
 public:
   WDecoCheat(WGuiBase * _it);
   virtual bool Visible();
   bool Selectable();
   virtual void Reload();
protected:
  bool bVisible;
};

class WGuiButton: public WGuiDeco{
public:
  WGuiButton( WGuiBase* _it, int _controller, int _control,  JGuiListener * jgl);
  virtual void updateValue();
  virtual void Update(float dt);
  virtual bool Selectable() {return Visible();};
  virtual PIXEL_TYPE getColor(int type);
protected:
  int control, controller;
  JGuiListener * mListener;
};

class WGuiHeader:public WGuiItem{
 public:

  WGuiHeader(string _displayValue): WGuiItem(_displayValue) {};
  virtual bool Selectable() {return false;};
  virtual void Render();

};

class WDecoStyled: public WGuiDeco{
public:
  WDecoStyled(WGuiItem * _it) : WGuiDeco(_it) {mStyle=DS_DEFAULT;};
  PIXEL_TYPE getColor(int type);
  void subBack(WGuiBase * item);
   enum {
    DS_DEFAULT = (1<<0),
    DS_COLOR_BRIGHT = (1<<1),
    DS_COLOR_DARK = (1<<2),
    DS_STYLE_EDGED = (1<<4),
    DS_STYLE_BACKLESS = (1<<5), 
   };

  u8 mStyle;
};

class WGuiMenu: public WGuiItem{
public:

  virtual ~WGuiMenu();
  WGuiMenu(u32 next, u32 prev);

  virtual void Render();
  virtual void Reload();
  virtual void Update(float dt);
  virtual void ButtonPressed(int controllerId, int controlId);
  virtual void Add(WGuiBase* item); //Remember, does not set X & Y of items automatically.
  virtual void confirmChange(bool confirmed);
  virtual bool Leaving(u32 key);
  virtual void Entering(u32 key);
  virtual void subBack(WGuiBase * item);
  

  WGuiBase * Current();
  virtual void nextItem(); 
  virtual void prevItem();
  virtual bool isModal();
  virtual void setModal(bool val);

  void setData();
  
protected:
  u32 buttonNext, buttonPrev;
  vector<WGuiBase*> items;
  int currentItem;
  u32 held;
  float duration;
};

class WGuiList: public WGuiMenu{
 public:
  WGuiList(string name, WDataSource * syncme = NULL);

  string failMsg;

  virtual void Render();
  virtual void confirmChange(bool confirmed);
  virtual void ButtonPressed(int controllerId, int controlId);
  virtual void setData(); 
  
  virtual void nextItem(); 
  virtual void prevItem();

  WGuiBase * operator[](int);
protected:
  WDataSource * sync;
  bool mFocus;
};

class WGuiTabMenu: public WGuiMenu {
 public:
   WGuiTabMenu() : WGuiMenu(PSP_CTRL_RTRIGGER,PSP_CTRL_LTRIGGER) {};
  virtual void Render();
  virtual void Add(WGuiBase * it);
  void save();
};

class OptionInteger:public OptionItem{
 public:
  int value;              //Current value.
  int defValue;           //Default value.
  string strDefault;      //What to call the default value.
  int maxValue, increment, minValue;

  OptionInteger(int _id, string _displayValue, int _maxValue = 1, int _increment = 1, int _defV = 0, string _sDef = "", int _minValue = 0);

  virtual void Reload() {if(id != INVALID_OPTION) value = options[id].number;};
  virtual bool Changed() {return value != options[id].number;};
  virtual void Render();
  virtual void setData();
  virtual void updateValue(){value+=increment; if (value>maxValue) value=minValue;};
};

class OptionSelect:public OptionItem{
 public:
  size_t value;
  vector<string> selections;

  virtual void addSelection(string s);
  OptionSelect(int _id, string _displayValue): OptionItem(_id, _displayValue) {value = 0;};
  virtual void Reload(){initSelections();};
  virtual void Render();
  virtual bool Selectable();
  virtual void Entering(u32 key);
  virtual bool Changed() {return (value != prior_value);};
  virtual void setData();
  virtual void initSelections();
  virtual void updateValue(){value++; if (value > selections.size() - 1) value=0;};
 protected:
   size_t prior_value;
};

class OptionLanguage: public OptionSelect{
 public:
  OptionLanguage(string _displayValue);
  
  virtual void addSelection(string s) {addSelection(s,s);};
  virtual void addSelection(string s,string show);
  virtual void initSelections();
  virtual void confirmChange(bool confirmed);
  virtual void Reload();
  virtual bool Visible();
  virtual bool Selectable();
  virtual void setData();
protected:
  vector<string> actual_data;
};


class OptionDirectory:public OptionSelect{
 public:
  virtual void Reload();
  OptionDirectory(string root, int id, string displayValue, const string type);
 protected:
  const string root;
  const string type;
};

class OptionTheme:public OptionDirectory{
 private:
  static const string DIRTESTER;
 public:
  OptionTheme();
  JQuad * getImage();
  virtual void updateValue();
  virtual float getHeight();
  virtual void Render();
  virtual void confirmChange(bool confirmed);
  virtual bool Visible();

protected:
  string author;
  bool bChecked;
};

class OptionProfile:public OptionDirectory{
 private:
  static const string DIRTESTER;
 public:
  OptionProfile(GameApp * _app,  JGuiListener * jgl);
  virtual void addSelection(string s);
  virtual bool Selectable() {return canSelect;};
  virtual bool Changed() {return (initialValue != value);};
  virtual void Entering(u32 key);
  virtual void Reload();
  virtual void Render();
  virtual void confirmChange(bool confirmed);
  virtual void Update(float dt);
  virtual void updateValue();
  void populate();
private:  
  GameApp * app;  
  JGuiListener * listener;
  bool canSelect;
  string preview;
  size_t initialValue;
};
#endif
