#ifndef _OPTION_ITEM_H_
#define _OPTION_ITEM_H_

#include <JGui.h>
#include <vector>
#include <string>
#include "../include/GameApp.h"
#include "../include/GameStateOptions.h"
#include "../include/GameOptions.h"
#include "../include/WFilter.h"
#include "../include/WDataSrc.h"
#include "../include/WGui.h"

using std::string;

#define MAX_OPTION_TABS 5
#define MAX_ONSCREEN_OPTIONS 8
#define OPTION_CENTER 4
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

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
  virtual void Entering(JButton key);
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
  virtual void Entering(JButton key);
  virtual void Reload();
  virtual void Render();
  virtual void confirmChange(bool confirmed);
  virtual void updateValue();
  void populate();
private:  
  GameApp * app;  
  JGuiListener * listener;
  bool canSelect;
  string preview;
  size_t initialValue;
};

class OptionKey : public WGuiItem, public KeybGrabber {
 public:
  OptionKey(GameStateOptions* g, LocalKeySym, JButton);
  LocalKeySym from;
  JButton to;
  virtual void Render();
  virtual void Update(float);
  virtual void Overlay();
  virtual bool CheckUserInput(JButton key);
  virtual void KeyPressed(LocalKeySym key);
  virtual bool isModal();
  virtual void ButtonPressed(int controllerId, int controlId);
  virtual bool Visible();
  virtual bool Selectable();
 protected:
  bool grabbed;
  GameStateOptions* g;
  SimpleMenu* btnMenu;
};
#endif
