#ifndef _GUIHAND_H_
#define _GUIHAND_H_

#include "GuiLayers.h"
#include "WEvent.h"
#include "MTGGameZones.h"
#include "CardGui.h"
#include "GuiHand.h"
#include "CardSelector.h"

class GuiHand;

struct HandLimitor : public Limitor
{
  GuiHand* hand;
  virtual bool select(Target*);
  virtual bool greyout(Target*);

  HandLimitor(GuiHand* hand);
};

class GuiHand : public GuiLayer
{
 public:
  static const float ClosedRowX;
  static const float LeftRowX;
  static const float RightRowX;

  static const float OpenX;
  static const float ClosedX;

 protected:
  const MTGHand* hand;
  JQuad *back;
  vector<CardView*> cards;
  CardSelector* cs;

 public:
  GuiHand(CardSelector* cs, MTGHand* hand);
  ~GuiHand();
  void Update(float dt);
  bool isInHand(CardView*);

  friend struct HandLimitor;
};

class GuiHandOpponent : public GuiHand
{
 public:
  GuiHandOpponent(CardSelector* cs, MTGHand* hand);
  virtual void Render();
  virtual int receiveEventPlus(WEvent* e);
  virtual int receiveEventMinus(WEvent* e);
};

class GuiHandSelf : public GuiHand
{
 protected:
  enum
  {
    Open,
    Closed
  } state;
  Pos backpos;

 public:
  GuiHandSelf(CardSelector* cs, MTGHand* hand);
  ~GuiHandSelf();
  virtual int receiveEventPlus(WEvent* e);
  virtual int receiveEventMinus(WEvent* e);

  void Repos();
  bool CheckUserInput(u32 key);
  virtual void Render();
  void Update(float dt);
  float LeftBoundary();

  HandLimitor* limitor;
};

#endif // _GUIHAND_H_
