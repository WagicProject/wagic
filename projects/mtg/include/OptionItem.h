#ifndef _OPTION_ITEM_H_
#define _OPTION_ITEM_H_

#include <JGui.h>
#include <vector>
#include <string>
#include "../include/GameApp.h"
#include "../include/GameOptions.h"

using std::string;

#define MAX_OPTION_TABS 5
#define MAX_OPTION_ITEMS 20
#define MAX_ONSCREEN_OPTIONS 8
#define OPTION_CENTER 4

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
  virtual PIXEL_TYPE getColor(int type)=0;
  
  virtual void Entering(u32 key)=0;  
  virtual bool Leaving(u32 key)=0;

  virtual void Update(float dt)=0;
  virtual void updateValue(){};
  virtual void Render()=0;
  virtual void setData()=0;
  virtual void ButtonPressed(int controllerId, int controlId){};
  virtual void Reload(){};
  virtual void Overlay(){};

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
};

//This is our base class for concrete items. 
class WGuiItem: public WGuiBase{
public:
  virtual void Entering(u32 key);  
  virtual bool Leaving(u32 key);
  virtual void Update(float dt);
  virtual void Render();

  WGuiItem(string _display);
  virtual ~WGuiItem() {};


  virtual PIXEL_TYPE getColor(int type);
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

class WGuiSplit: public WGuiItem{
public:
  WGuiSplit(WGuiBase* _left,WGuiBase* _right);
  ~WGuiSplit();

  virtual void Reload();
  virtual void Overlay();
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
  ~WDecoConfirm();

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


class WGuiImage: public WGuiItem{
public:
  WGuiImage(string _file, int _w, int _h, int _margin);
  virtual bool Selectable() {return false;};
  virtual JQuad * getImage();
  virtual void Render();
  virtual float getHeight();

protected:
  bool exact;
  int margin;
  int imgW, imgH;
  string filename;
};

class WGuiText:public WGuiItem {
 public:
  WGuiText(string _displayValue): WGuiItem(_displayValue) {};
  virtual bool Selectable() {return false;};
  virtual void Render();
};

class WGuiHeader:public WGuiItem{
 public:
  WGuiHeader(string _displayValue): WGuiItem(_displayValue) {};

  virtual bool Selectable() {return false;};
  virtual void Render();
};


class WGuiList: public WGuiItem{
 public:
  WGuiList(string name);
  ~WGuiList();

  string failMsg;
  int nbitems;
  int current;

  virtual bool hasFocus() {return mFocus;};
  virtual void setFocus(bool bFocus) {mFocus = bFocus;};
  virtual bool Leaving(u32 key);
  virtual void Entering(u32 key);
  virtual void Render();
  virtual void confirmChange(bool confirmed);
  virtual void renderBack(WGuiBase * it);
  virtual void Reload();
  virtual void ButtonPressed(int controllerId, int controlId);
  virtual void Update(float dt);
  virtual void setData(); 
  virtual bool isModal();
  virtual void setModal(bool val);
  
  void Add(WGuiBase * item);
  WGuiBase * Current();
  void nextOption();
  void prevOption();

  WGuiBase * operator[](int);
protected:
  bool mFocus;
  WGuiBase * listItems[MAX_OPTION_ITEMS];
};

class WGuiMenu{
public:

  virtual ~WGuiMenu();
  WGuiMenu(u32 next, u32 prev);

  virtual void Render();
  virtual void Reload();
  virtual void Update(float dt);
  virtual void ButtonPressed(int controllerId, int controlId);
  virtual void Add(WGuiBase* item);
  virtual void confirmChange(bool confirmed);
  
  WGuiBase * Current();
  void nextItem();
  void prevItem();
  void setData();
  
protected:
  u32 buttonNext, buttonPrev;
  vector<WGuiBase*> items;
  int currentItem;
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
  int maxValue, increment;

  OptionInteger(int _id, string _displayValue, int _maxValue = 1, int _increment = 1, int _defV = 0, string _sDef = "");

  virtual void Reload() {if(id != INVALID_OPTION) value = options[id].number;};
  virtual bool Changed() {return value != options[id].number;};
  virtual void Render();
  virtual void setData();
  virtual void updateValue(){value+=increment; if (value>maxValue) value=0;};
};

class OptionString:public OptionItem{
 public:
  string value;
  OptionString(int _id, string _displayValue);

  virtual void Render();
  virtual void setData();
  virtual void updateValue();
  virtual bool Changed() {return value != options[id].str;};
  virtual void Reload() {if(id != INVALID_OPTION) value = options[id].str;};
  bool bShowValue;
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

class OptionDirectory:public OptionSelect{
 public:
  virtual void Reload();
  OptionDirectory(string _root, int _id, string _displayValue);
private:
  string root;
};

class OptionTheme:public OptionDirectory{
 public:
  OptionTheme();
  JQuad * getImage();
  virtual float getHeight();
  virtual void Render();
  virtual void confirmChange(bool confirmed);
  virtual bool Visible();
};

class OptionProfile:public OptionDirectory{
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