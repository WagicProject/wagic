#include "../include/ActionElement.h"
#include "../include/MTGCardInstance.h"
#include "../include/Targetable.h"
#include "../include/TargetChooser.h"

ActionElement::ActionElement(int id):JGuiObject(id){
  activeState = INACTIVE;
  modal = 0;
  waitingForAnswer = 0;
  currentPhase = -1;
  newPhase = -1;
  tc = NULL;
}

/*
  void ActionElement::RenderMessageBackground(float y0, int _height){
  float height = _height;
  PIXEL_TYPE colors_up[] =
  {
  ARGB(0,255,255,255),
  ARGB(0,255,255,255),
  ARGB(128,255,255,255),
  ARGB(128,255,255,255)
  };

  PIXEL_TYPE colors_down[] =
  {
  ARGB(128,255,255,255),
  ARGB(128,255,255,255),
  ARGB(0,255,255,255),
  ARGB(0,255,255,255)
  };

  JRenderer * renderer = JRenderer::GetInstance();
  renderer->FillRect(0,y0,SCREEN_WIDTH,height/2,colors_up);
  renderer->FillRect(0,y0+height/2,SCREEN_WIDTH,height/2,colors_down);
  //  mEngine->DrawLine(0,y0,SCREEN_WIDTH,y0,ARGB(128,255,255,255));
  //  mEngine->DrawLine(0,y0+height,SCREEN_WIDTH,y0+height,ARGB(128,255,255,255));
  }*/

int ActionElement::getActivity(){

  return activeState;
}


int ActionElement::isReactingToTargetClick(Targetable * object){
  if (object && object->typeAsTarget() == TARGET_CARD) return isReactingToClick((MTGCardInstance *)object);
  return 0;
}

int ActionElement::reactToTargetClick(Targetable * object){
  if (object->typeAsTarget() == TARGET_CARD) return reactToClick((MTGCardInstance *)object);
  return 0;
}
