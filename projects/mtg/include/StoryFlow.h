#ifndef _STORYFLOW_H_
#define _STORYFLOW_H_

#include <string>
#include <map>
#include <vector>
using namespace std;
#include "../../../JGE/src/tinyxml/tinyxml.h"
#include <JGui.h>
class GameObserver;
#define CAMPAIGNS_FOLDER "Res/campaigns/"


class StoryDialogElement:public JGuiObject {
public:
  float mX;
  float mY;
 StoryDialogElement(float x, float y, int id = 0);
   void Entering(){};
   bool Leaving(JButton key) {return false;};
   bool ButtonPressed() {return false;};
   bool hasFocus() {return false;};
   virtual float getHeight() = 0;
};

class StoryText:public StoryDialogElement {
public : 
  string text;
  int align;
  int font;
StoryText(string text, float mX, float mY, string align = "center", int font = 0, int id = 0);

  void Render();
  void Update(float dt);
  virtual ostream& toString(ostream& out) const;
  float getHeight();
};
class StoryImage:public StoryDialogElement {
public :
  string img;
StoryImage(string img, float mX, float mY);
  void Render();
  void Update(float dt);
  virtual ostream& toString(ostream& out) const;
  float getHeight();
};

class StoryChoice:public StoryText {
public:
  string pageId;

  bool mHasFocus;
  float mScale;
  float mTargetScale;
  StoryChoice(string id, string text, int JGOid, float mX, float mY,  string _align, int _font, bool hasFocus);
  void Render();
  void Update(float dt);

  void Entering();
  bool Leaving(JButton key);
  bool ButtonPressed();
  bool hasFocus();
  virtual ostream& toString(ostream& out) const;
  float getHeight();
};

class StoryFlow;
class StoryPage {
public: 
  StoryFlow * mParent;
  StoryPage(StoryFlow * mParent);
  virtual void Update(float dt)=0;
  virtual void Render()=0;
  virtual ~StoryPage(){};
};

class StoryDialog:public StoryPage, public JGuiListener,public JGuiController {
private:
  vector<StoryDialogElement *>graphics;
  string safeAttribute(TiXmlElement* element, string attribute);
  void RenderElement(StoryDialogElement * elmt);
public:
  StoryDialog(TiXmlElement* el,StoryFlow * mParent);
  ~StoryDialog();
  void Update(float dt);
  void Render();
  void ButtonPressed(int,int);

  static float currentY;
  static float previousY;
};


class Rules;
class StoryDuel:public StoryPage {
public:
  string pageId;
  string onWin, onLose;
  GameObserver * game;
  Rules * rules;
  StoryDuel(TiXmlElement* el,StoryFlow * mParent);
  virtual ~StoryDuel();
  void Update(float dt);
  void Render();
  void init();
};

class StoryFlow{
private:
  map<string,StoryPage *>pages;
  bool parse(string filename);
  StoryPage * loadPage(TiXmlElement* element);
public: 
  string currentPageId;
  string folder;
  StoryFlow(string folder);
  ~StoryFlow();

  bool gotoPage(string id);
  void Update(float dt);
  void Render();
};


#endif
