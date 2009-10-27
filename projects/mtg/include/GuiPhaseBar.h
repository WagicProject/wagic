#ifndef _GUIPHASEBAR_H_
#define _GUIPHASEBAR_H_

#include "GuiLayers.h"
#include "PhaseRing.h"
#include "WEvent.h"

class GuiPhaseBar : public GuiLayer
{
  static const unsigned Width = 28;
  static const unsigned Height = Width;
  static const unsigned Phases = 12;

 protected:
  Phase* phase;
  float angle;

 public:
  GuiPhaseBar();
  ~GuiPhaseBar();
  void Update(float dt);
  virtual void Render();
  virtual int receiveEventMinus(WEvent * e);
};

#endif // _GUIPHASEBAR_H_
