#ifndef _GAME_STATE_TRANSITIONS_H_
#define _GAME_STATE_TRANSITIONS_H_

#include <JGE.h>
#include <JGui.h>
#include "GameState.h"

class TransitionBase: public GameState, public JGuiListener{
public:
  TransitionBase(GameApp* parent, GameState* _from, GameState* _to, float duration);
  ~TransitionBase();
  virtual void Start();
  virtual void End();

  virtual bool Finished() {return (mElapsed >= mDuration);};
  virtual void Update(float dt);
  virtual void Render() = 0;
  virtual void ButtonPressed(int controllerId, int controlId);

  float mElapsed;
  float mDuration;
  GameState* from;
  GameState* to;
  bool bAnimationOnly; //Does not call start or end on subordinates.
};

class TransitionFade: public TransitionBase {
public:
  TransitionFade(GameApp* p, GameState* f, GameState* t, float dur, bool reversed);
  virtual void Render();
  bool mReversed;
};

#endif
