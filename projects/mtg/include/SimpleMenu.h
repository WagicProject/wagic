/*
  A class for very simple menus structure
*/
#ifndef _SIMPLEMENU_H_
#define _SIMPLEMENU_H_

#include <JGui.h>
#include <JLBFont.h>
#include <string>

class SimpleMenu:public JGuiController{
 private:
  int mHeight, mWidth, mX, mY;
  JLBFont* mFont;
  std::string title;
  int displaytitle;
 public:
  SimpleMenu(int id, JGuiListener* listener, JLBFont* font, int x, int y, int width, const char * _title = NULL);
  void Render();
  void Add(int id, const char * Text);
};


#endif
