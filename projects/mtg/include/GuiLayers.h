#ifndef _GUI_LAYERS_H_
#define _GUI_LAYERS_H_

#define MAX_GUI_LAYERS 6

#define DIR_DOWN 1
#define DIR_UP 2
#define DIR_LEFT 3
#define DIR_RIGHT 4

#include <JGui.h>


class GameObserver;
class Player;

class GuiLayer: public JGuiController{
 protected:
  int modal;
  GameObserver * game;
 public:
  virtual void resetObjects();
	int hasFocus;
	int getMaxId();
	void RenderMessageBackground(float x0, float y0, float width, int height);
  void RenderMessageBackground(float y0, int height);
  GuiLayer(int id, GameObserver* _game);
  virtual int isModal();
  void setModal(int _modal);
  virtual ~GuiLayer();
  virtual void Update(float dt);
  virtual void CheckUserInput(float dt){};
  virtual int unstopableRenderInProgress(){return 0;};
  int getIndexOf(JGuiObject * object);
  JGuiObject * getByIndex (int index);
  virtual void Render(){JGuiController::Render();};
	int empty(){
		if (mCount) return 0;
		return 1;
	};
};

class GuiLayers{
 protected:
  int nbitems;
  GuiLayer * objects[MAX_GUI_LAYERS];
 public:
  GuiLayers();
  void Update(float dt, Player * player);
  void Render();
  void Add(GuiLayer * layer);
  void Remove();
  int unstopableRenderInProgress();
  ~GuiLayers();

};

#endif
