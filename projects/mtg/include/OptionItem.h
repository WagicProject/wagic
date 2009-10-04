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

#define OPTIONS_SUBMODE_NORMAL 0
#define OPTIONS_SUBMODE_RELOAD 1
#define OPTIONS_SUBMODE_PROFILE 2
#define OPTIONS_SUBMODE_MODE 3
#define OPTIONS_SUBMODE_THEME 4

class OptionItem {
public:
  string displayValue;
  int id;
  int hasFocus;
  bool canSelect;
  bool bHidden;
  float x, y;
  float width, height;
  virtual ostream& toString(ostream& out)const;

  OptionItem( int _id,  string _displayValue);
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

  OptionInteger(int _id, string _displayValue, int _maxValue = 1, int _increment = 1, int _defV = 0, string _sDef = "");

  virtual void Reload() {if(id != INVALID_OPTION) value = options[id].number;};
  virtual void Render();
  virtual void setData();
  virtual void updateValue(){value+=increment; if (value>maxValue) value=0;};
  virtual ostream& toString(ostream& out) const;
};

class OptionString:public OptionItem{
 public:
  string value;
  OptionString(int _id, string _displayValue);

  virtual void Render();
  virtual void setData();
  virtual void updateValue();
  virtual void Reload() {if(id != INVALID_OPTION) value = options[id].str;};
  virtual ostream& toString(ostream& out) const;
  bool bShowValue;
};

class OptionNewProfile:public OptionString{
 public:
   OptionNewProfile(string _displayValue) :  OptionString(INVALID_OPTION, _displayValue) {bShowValue=false;};
   virtual void updateValue();
   virtual void Update(float dt);
   virtual int Submode();
   bool bChanged;
};

class OptionHeader:public OptionItem{
 public:
  OptionHeader(string _displayValue): OptionItem(INVALID_OPTION, _displayValue) { canSelect=false;};
  virtual void Render();
  virtual void setData() {};
  virtual void updateValue() {};
};

class OptionText:public OptionItem{
 public:
  OptionText(string _displayValue): OptionItem(INVALID_OPTION, _displayValue) { canSelect=false;};
  virtual void Render();
  virtual void setData() {};
  virtual void updateValue() {};
};

class OptionSelect:public OptionItem{
 public:
  size_t value;
  vector<string> selections;

  virtual void addSelection(string s);
  OptionSelect(int _id, string _displayValue): OptionItem(_id, _displayValue) {value = 0;};
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
  OptionDirectory(string _root, int _id, string _displayValue);
private:
  string root;
};

class OptionTheme:public OptionDirectory{
 public:
  OptionTheme();
};

class OptionVolume: public OptionInteger{
 public:
   OptionVolume(int _id, string _displayName, bool _bMusic = false);
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
  unsigned index;
 public:
  OptionEnum(int id, string displayValue);
  virtual void Reload();
  virtual void Render();
  virtual void setData();
  virtual void updateValue();
  //ourDefined is a virtual wrapper for getDefinition()
  virtual EnumDefinition * ourDefined() const {return getDefinition();}; 

  static EnumDefinition * getDefinition();
  virtual ostream& toString(ostream& out) const;
};

class OptionClosedHand : public OptionEnum {
 public:
   friend class GameSettings;
  enum { INVISIBLE = 0, VISIBLE = 1 };
  OptionClosedHand(int id, string displayValue);

  static EnumDefinition * getDefinition();
  EnumDefinition * ourDefined() const { return getDefinition();};
 
private:
  static EnumDefinition * definition;
};
class OptionHandDirection : public OptionEnum {
 public:
   friend class GameSettings;
  enum { VERTICAL = 0, HORIZONTAL = 1};
  OptionHandDirection(int id, string displayValue);
  
  static EnumDefinition * getDefinition();
  EnumDefinition * ourDefined() const { return getDefinition();};

private:
  static EnumDefinition * definition;
};

class OptionManaDisplay : public OptionEnum {
 public:
   friend class GameSettings;
  enum { DYNAMIC = 0, STATIC = 1, BOTH = 2};
  OptionManaDisplay(int id, string displayValue);
  
  static EnumDefinition * getDefinition();
  EnumDefinition * ourDefined() const { return getDefinition();};

private:
  static EnumDefinition * definition;
};

#endif
