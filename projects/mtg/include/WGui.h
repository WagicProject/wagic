#ifndef _WGUI_H_
#define _WGUI_H_

class hgeDistortionMesh;
class GameStateOptions;

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
    BACK_ALERT,
    BACK_HEADER,
    BACK_FAIL,
    BACK_TAB,
  };
};

struct WDistort {
  WDistort();
  WDistort(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);
  float & operator[](int p);
protected:
  float xy[8];
};

//Complete item interface
class WGuiBase: public JGuiListener {
public:
  WGuiBase() {};
  virtual ~WGuiBase() {};

  virtual bool Selectable() {return true;};
  virtual bool isModal() {return false;};
  virtual bool Visible() {return true;};
  
  virtual bool Changed() {return false;};
  virtual void confirmChange(bool confirmed) {};
  virtual PIXEL_TYPE getColor(int type);
  virtual float getMargin(int type) {return 4;};
  
  virtual void Entering(JButton key)=0;  
  virtual bool Leaving(JButton key)=0;

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
  virtual string getDisplay() const {return "";};  
  virtual float minWidth(){return getWidth();};
  virtual float minHeight(){return getHeight();};
  
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

  virtual bool CheckUserInput(JButton key) {return false;};
};

//This is our base class for concrete items. 
class WGuiItem: public WGuiBase{
public:
  virtual void Entering(JButton key);  
  virtual bool Leaving(JButton key);
  virtual bool CheckUserInput(JButton key);
  virtual void Update(float dt) {};
  virtual void Render();

  WGuiItem(string _display, u8 _mF = 0);
  virtual ~WGuiItem() {};

  string _(string input); //Override global with our flag checker.
  
  virtual void setData(){};

  virtual bool hasFocus() {return mFocus;};
  virtual void setFocus(bool bFocus) {mFocus = bFocus;};

  virtual string getDisplay() const {return displayValue;};
  virtual void setDisplay(string s){displayValue=s;};  

  virtual int getId() {return INVALID_ID;};
  virtual float getX() {return x;};
  virtual float getY() {return y;};
  virtual float getWidth() {return width;};
  virtual float getHeight() {return height;}; 
  virtual float minWidth();
  virtual float minHeight();
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

class WGuiImage: public WGuiItem{
public:
  WGuiImage(WDataSource * wds, float _w = 0, float _h = 0, int _margin = 0);
  virtual bool Selectable() {return false;};
  virtual void Render();
  virtual float getHeight();
  virtual void imageScale(float _w, float _h);
  virtual void setSource(WDataSource *s) {source = s;};
protected:
  int margin;
  float imgW, imgH;
  WDataSource * source;};

class WGuiCardImage: public WGuiImage{
public:
  WGuiCardImage(WDataSource * wds, bool _thumb=false);
  virtual void Render();
  WSyncable mOffset;
protected:
  bool bThumb;
};

class WGuiCardDistort: public WGuiCardImage{
public:
  WGuiCardDistort(WDataSource * wds, bool _thumb=false, WDataSource * _distort=NULL);
  ~WGuiCardDistort();
  virtual void Render();
  WDistort xy; 
protected:
  hgeDistortionMesh* mesh;
  WDataSource * distortSrc;
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
 
  virtual void Entering(JButton key)       {it->Entering(key);};  
  virtual bool Leaving(JButton key)        {return it->Leaving(key);};
  virtual void Update(float dt) {it->Update(dt);};
  virtual void updateValue()    {it->updateValue();};
  virtual void Reload()         {it->Reload();};
  virtual void Overlay()        {it->Overlay();};
  virtual void Underlay()        {it->Underlay();};
  virtual void Render()         {it->Render();};
  virtual void setData()        {it->setData();};

  virtual void ButtonPressed(int controllerId, int controlId) {it->ButtonPressed(controllerId, controlId);};

  virtual bool hasFocus()             {return it->hasFocus();};
  virtual string getDisplay() const   {return it->getDisplay();};
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
  virtual bool CheckUserInput(JButton key) {return it->CheckUserInput(key);};
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

  virtual void Entering(JButton key);  
  virtual bool Leaving(JButton key);  
  virtual bool CheckUserInput(JButton key);

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
  virtual void Entering(JButton key);
  virtual bool Leaving(JButton key);
  virtual void Update(float dt);
  virtual void Overlay();
  virtual void ButtonPressed(int controllerId, int controlId);
  virtual bool CheckUserInput(JButton key);

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
  virtual bool CheckUserInput(JButton key);
  virtual bool Selectable() {return Visible();};
  virtual PIXEL_TYPE getColor(int type);
  virtual int getControlID() {return control;};
  virtual int getControllerID() {return controller;};
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
    DS_STYLE_ALERT = (1<<3), 
    DS_STYLE_EDGED = (1<<4),
    DS_STYLE_BACKLESS = (1<<5), 
   };

  u8 mStyle;
};

class WGuiMenu: public WGuiItem{
public:
  friend class WGuiFilters;
  virtual ~WGuiMenu();
  WGuiMenu(JButton next, JButton prev, bool mDPad = false, WSyncable * syncme=NULL);

  virtual void Render();
  virtual void Reload();
  virtual void Update(float dt);
  virtual void ButtonPressed(int controllerId, int controlId);
  virtual void Add(WGuiBase* item); //Remember, does not set X & Y of items automatically.
  virtual void confirmChange(bool confirmed);
  virtual bool Leaving(JButton key);
  virtual void Entering(JButton key);
  virtual void subBack(WGuiBase * item);
  virtual bool CheckUserInput(JButton key);
  WGuiBase * Current();
  virtual int getSelected() {return currentItem;};
  virtual bool nextItem(); 
  virtual bool prevItem();
  virtual bool isModal();
  virtual void setModal(bool val);

  void setData();
  
protected:
  virtual void syncMove();
  virtual bool isButtonDir(JButton key, int dir); //For the DPad override.
  JButton buttonNext, buttonPrev;
  bool mDPad;
  vector<WGuiBase*> items;
  int currentItem;
  JButton held;
  WSyncable * sync;
  float duration;
};

class WGuiList: public WGuiMenu{
 public:
  WGuiList(string name, WSyncable * syncme = NULL);

  string failMsg;

  virtual void Render();
  virtual void confirmChange(bool confirmed);
  virtual void ButtonPressed(int controllerId, int controlId);
  virtual void setData(); 
  WGuiBase * operator[](int);
protected:
  bool mFocus;
};
class WGuiTabMenu: public WGuiMenu {
 public:
   WGuiTabMenu() : WGuiMenu(JGE_BTN_NEXT, JGE_BTN_PREV) {};
  virtual void Render();
  virtual void Add(WGuiBase * it);
  void save();
};
class WGuiListRow: public WGuiList{
 public:
   WGuiListRow(string n, WSyncable * s = NULL);
   virtual void Render();
};

class WGuiFilters: public WGuiItem {
public:
  friend class WGuiFilterItem;
  WGuiFilters(string header, WSrcCards * src);
  ~WGuiFilters();
  bool CheckUserInput(JButton key);
  string getCode(); //For use in filter factory.
  void Update(float dt);
  void Render();
  void Entering(JButton key);
  void addColumn();
  void recolorFilter(int color);
  bool isAvailable(int type);
  bool isAvailableCode(string code);
  bool Finish(bool emptyset = false); //Returns true if card set reasonably expected to be changed.
  bool isFinished() {return bFinished;};
  void ButtonPressed(int controllerId, int controlId);
  void buildList();
protected:
  void clearArgs();
  void addArg(string display, string code);
  vector< pair<string,string> > tempArgs; //TODO FIXME this is inefficient
  bool bFinished;
  int recolorTo;
  WSrcCards* source;
  SimpleMenu* subMenu;
  WGuiList * list;
};

class WGuiFilterItem: public WGuiItem {
public:
  friend class WGuiFilters;
  friend struct WLFiltersSort;
  WGuiFilterItem(WGuiFilters * parent);
  void updateValue();
  void ButtonPressed(int controllerId, int controlId);
  string getCode();
  bool isModal();
  enum {
    STATE_UNSET,
    STATE_CHOOSE_TYPE,
    STATE_CHOOSE_VAL,
    STATE_FINISHED,
    STATE_REMOVE,
    STATE_CANCEL,
    BEGIN_FILTERS = 0,
    FILTER_SET = BEGIN_FILTERS,
    FILTER_ALPHA,
    FILTER_RARITY,
    FILTER_COLOR,
    FILTER_PRODUCE,
    FILTER_TYPE,
    FILTER_BASIC,
    FILTER_CMC,
    FILTER_POWER,
    FILTER_TOUGH,
    END_FILTERS
  };
protected:
  string mCode;
  int filterType;
  int filterVal;
  int mState;
  bool mNew;
  WGuiFilters * mParent;
};

struct WListSort{
  virtual bool operator()(const WGuiBase*l, const WGuiBase*r);
};

struct WLFiltersSort{
  bool operator()(const WGuiBase*l, const WGuiBase*r);
};

class WGuiKeyBinder : public WGuiList {
 public:
  WGuiKeyBinder(string name, GameStateOptions* parent);
  virtual bool isModal();
  virtual bool CheckUserInput(JButton);
 protected:
  GameStateOptions* parent;
  bool modal;
};

#endif
