//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
//
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
//
//-------------------------------------------------------------------------------------


#ifndef _JGUI_H
#define _JGUI_H

#include <ostream>
#include "JGE.h"
#include "JSprite.h"

#define MAX_GUIOBJECT			64

#define JGUI_STYLE_LEFTRIGHT	0x01
#define JGUI_STYLE_UPDOWN		0x02
#define JGUI_STYLE_WRAPPING		0x04

#define JGUI_INITIAL_DELAY		0.4
#define JGUI_REPEAT_DELAY		0.2

class JGuiListener
{
 public:
  virtual ~JGuiListener() {}
  virtual void ButtonPressed(int controllerId, int controlId) = 0;
};


class JGuiObject
{
 protected:
  static JGE*	mEngine;

 private:
  int mId;


 public:
  JGuiObject(int id);
  virtual ~JGuiObject();

  virtual void Render() = 0;
  virtual std::ostream& toString(std::ostream&) const = 0;
  virtual void Update(float dt);

  virtual void Entering();			// when focus is transferring to this obj
  virtual bool Leaving(u32 key);		// when focus is transferring away from this obj, true to go ahead
  virtual bool ButtonPressed();		// action button pressed, return false to ignore

  int GetId();
};


class JGuiController
{
 protected:
  static JGE*	mEngine;

  int mId;
  bool mActive;


  u32	mActionButton;
  int mCurr;
  int mStyle;

  JSprite* mCursor;
  bool mShowCursor;
  int mCursorX;
  int mCursorY;

  int mBgX;
  int mBgY;
  const JTexture* mBg;
  PIXEL_TYPE mShadingColor;
  Rect* mShadingBg;

  JGuiListener* mListener;
  //int mKeyHoldTime;

 public:
  vector<JGuiObject*> mObjects;
  int mCount;

  JGuiController(int id, JGuiListener* listener);
  ~JGuiController();

  virtual void Render();
  virtual void Update(float dt);
  virtual bool CheckUserInput(u32 key);

  void Add(JGuiObject* ctrl);
  void RemoveAt(int i);
  void Remove(int id);
  void Remove(JGuiObject* ctrl);

  void SetActionButton(u32 button);
  void SetStyle(int style);
  void SetCursor(JSprite* cursor);

  bool IsActive();
  void SetActive(bool flag);

  //void SetImageBackground(const JTexture* tex, int x, int y);
  //void SetShadingBackground(int x, int y, int width, int height, PIXEL_TYPE color);
};

ostream& operator<<(ostream &out, const JGuiObject &j);

#endif
