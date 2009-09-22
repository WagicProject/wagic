#ifndef _OPTION_ITEM_H_
#define _OPTION_ITEM_H_

#include <JGui.h>
#include <vector>
#include <string>
#include "../include/GameApp.h"

using std::string;

#define MAX_OPTION_TABS 5
#define MAX_OPTION_ITEMS 20
#define MAX_ONSCREEN_OPTIONS 8
#define OPTION_CENTER 4

#define OPTIONS_SUBMODE_NORMAL 0
#define OPTIONS_SUBMODE_RELOAD 1
#define OPTIONS_SUBMODE_PROFILE 2
#define OPTIONS_SUBMODE_MODE 3
#define OPTIONS_SUBMODE_THEME 4

class OptionItem {
public:
  string displayValue, id;
  int hasFocus;
  bool canSelect;
  bool bHidden;
  float x, y;
  float width, height;
  virtual ostream& toString(ostream& out)const;

  OptionItem( string _id,  string _displayValue);
  virtual ~OptionItem() {};

  virtual bool Selectable() {return (canSelect && !bHidden);};
  virtual void Entering();
  virtual bool Leaving();
  virtual void Update(float dt);
  virtual void updateValue()=0;
  virtual void Reload(){};
  virtual void Render()=0;
  virtual void setData()=0;
  virtual int  Submode() {return OPTIONS_SUBMODE_NORMAL;};
  virtual void cancelSubmode() {};
  virtual void acceptSubmode() {};
};

class OptionInteger:public OptionItem{
 public:
  int value;              //Current value.
  int defValue;           //Default value.
  string strDefault;      //What to call the default value.
  int maxValue, increment;

  OptionInteger(string _id, string _displayValue, int _maxValue = 1, int _increment = 1, int _defV = 0, string _sDef = "");

  virtual void Reload() {if(id != "") value = options[id].number;};
  virtual void Render();
  virtual void setData();
  virtual void updateValue(){value+=increment; if (value>maxValue) value=0;};
  virtual ostream& toString(ostream& out) const;
};

class OptionString:public OptionItem{
 public:
  string value;
  OptionString(string _id, string _displayValue);

  virtual void Render();
  virtual void setData();
  virtual void updateValue();
  virtual void Reload() {if(id != "") value = options[id].str;};
  virtual ostream& toString(ostream& out) const;
  bool bShowValue;
};

class OptionNewProfile:public OptionString{
 public:
   OptionNewProfile(string _id, string _displayValue) :  OptionString(_id, _displayValue) {bShowValue=false;};
   virtual void updateValue();
   virtual void Update(float dt);
   virtual int Submode();
   bool bChanged;
};

class OptionHeader:public OptionItem{
 public:
  OptionHeader(string _displayValue): OptionItem("", _displayValue) { canSelect=false;};
  virtual void Render();
  virtual void setData() {};
  virtual void updateValue() {};
};

class OptionText:public OptionItem{
 public:
  OptionText(string _displayValue): OptionItem("", _displayValue) { canSelect=false;};
  virtual void Render();
  virtual void setData() {};
  virtual void updateValue() {};
};

class OptionSelect:public OptionItem{
 public:
  size_t value;
  vector<string> selections;

  virtual void addSelection(string s);
  OptionSelect(string _id, string _displayValue): OptionItem(_id, _displayValue) {value = 0;};
  virtual void Reload(){initSelections();};
  virtual void Render();
  virtual void setData();
  virtual void initSelections();
  virtual void updateValue(){value++; if (value > selections.size() - 1) value=0;};
  virtual ostream& toString(ostream& out) const;
};


class OptionDirectory:public OptionSelect{
 public:
  virtual void Reload();
  OptionDirectory(string _root, string _id, string _displayValue);
private:
  string root;
};

class OptionTheme:public OptionDirectory{
 public:
  OptionTheme();
};

class OptionVolume: public OptionInteger{
 public:
   OptionVolume(string _id, string _displayName, bool _bMusic = false);
   virtual void updateValue();
private:
   bool bMusic;
};

class OptionProfile:public OptionDirectory{
 public:
  OptionProfile(GameApp * _app);
  ~OptionProfile();
  virtual void addSelection(string s);    
  virtual bool Leaving();
  virtual void Entering();
  virtual void Render();
  virtual void Update(float dt);
  virtual void updateValue();
  virtual int  Submode();
  virtual void cancelSubmode();
  virtual void acceptSubmode();
  void populate();
private:  
  bool bCheck;
  JQuad * mAvatar;
  JTexture * mAvatarTex;
  GameApp * app;  
  string preview;
  size_t initialValue;
};

class OptionsList{
 public:
  string sectionName;
  string failMsg;
  int nbitems;
  int current;
  OptionsList(string name);
  ~OptionsList();
  bool Leaving();
  void Entering();
  void Render();
  void reloadValues();
  void Update(float dt);
  void Add(OptionItem * item);
  void save();
  int Submode();
  void acceptSubmode();
  void cancelSubmode();

  OptionItem * operator[](int);
private:
  OptionItem * listItems[MAX_OPTION_ITEMS];
};

class OptionsMenu
{
 public:
  JLBFont * mFont;
  OptionsList * tabs[MAX_OPTION_TABS];
  int nbitems;
  int current;

  OptionsMenu();
  ~OptionsMenu();

  bool isTab(string name);  
  int  Submode();
  void acceptSubmode();
  void cancelSubmode();
  void Render();
  void reloadValues();
  void Update(float dt);
  void Add(OptionsList * tab);
  void save();

};

class OptionEnum : public OptionItem {
 protected:
  typedef pair<int, string> assoc;
  unsigned index;
  vector<assoc> values;
 public:
 OptionEnum(string id, string displayValue) : OptionItem(id, displayValue), index(0) {};
  virtual void Reload();
  virtual void Render();
  virtual void setData();
  virtual void updateValue();
  virtual ostream& toString(ostream& out) const;
};

class OptionClosedHand : public OptionEnum {
 public:
  enum { INVISIBLE = 0, VISIBLE = 1 };
  OptionClosedHand(string id, string displayValue);
};
class OptionHandDirection : public OptionEnum {
 public:
  enum { VERTICAL = 0, HORIZONTAL = 1};
  OptionHandDirection(string id, string displayValue);
};

#endif
