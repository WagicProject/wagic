#ifndef _OPTION_ITEM_H_
#define _OPTION_ITEM_H_

#include <JGui.h>
#include <string>

using std::string;

class OptionItem:public JGuiObject{
 public:
  string displayValue, id;
  int value;
  int hasFocus;
  int maxValue, increment;
  float x, y;
  OptionItem(string _id, string _displayValue, int _maxValue = 1, int _increment = 1);

  ~OptionItem();
  virtual void Render();
  virtual void Update(float dt);
  virtual void Entering();
  virtual bool Leaving();
  void setData();
  virtual void updateValue(){value+=increment; if (value>maxValue) value=0;};
  virtual ostream& toString(ostream& out) const;
};

class OptionsList{
 public:
  OptionItem * options[20];
  int nbitems;
  int current;
  OptionsList();
  ~OptionsList();
  void Render();
  void Update(float dt);
  void Add(OptionItem * item);
  void save();
};

#endif
