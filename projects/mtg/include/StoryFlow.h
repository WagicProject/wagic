#ifndef _STORYFLOW_H_
#define _STORYFLOW_H_

#include <string>
#include <map>
#include <vector>
using namespace std;
#include "../../../JGE/src/TinyXML/tinyxml.h"
#include <JGui.h>
class GameObserver;
#define CAMPAIGNS_FOLDER "Res/campaigns/"

class StoryChoice:public JGuiObject {
public:
  string pageId;
  string text;
  int mX;
  int mY;
  bool mHasFocus;
  float mScale;
  float mTargetScale;
  StoryChoice(string id, string text, int JGOid, float mX, float mY, bool hasFocus);
  void Render();
  void Update(float dt);

  void Entering();
  bool Leaving(u32 key);
  bool ButtonPressed();
  bool hasFocus();
  virtual ostream& toString(ostream& out) const;
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
  string text;

public:
  StoryDialog(TiXmlElement* el,StoryFlow * mParent);
  void Update(float dt);
  void Render();
  void ButtonPressed(int,int);
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