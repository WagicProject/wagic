#ifndef _GUI_CARDS_CONTROLLER_H_
#define _GUI_CARDS_CONTROLLER_H_


#include "PlayGuiObjectController.h"

class GuiCardsController : public PlayGuiObjectController{
public:
	  GuiCardsController(int id, GameObserver* _game):PlayGuiObjectController(id, _game){};
};


#endif
