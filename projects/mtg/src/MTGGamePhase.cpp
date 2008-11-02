#include "../include/debug.h"
#include "../include/MTGGamePhase.h"


MTGGamePhase::MTGGamePhase(int id):ActionElement(id){
  animation = 0;
  currentState = -1;
	mFont= GameApp::CommonRes->GetJLBFont("graphics/f3");
  mFont->SetBase(0);	// using 2nd font

}


void MTGGamePhase::Render(){
  /*if (animation){
    RenderMessageBackground(10, 100);
    mFont->DrawString(MTGPhaseNames[currentState], SCREEN_WIDTH_F/2, 30, JGETEXT_CENTER);
  }*/
}


void MTGGamePhase::Update(float dt){

		int newState = GameObserver::GetInstance()->getCurrentGamePhase();
		if (newState != currentState){
			activeState = ACTIVE;
			animation = 1;
			currentState = newState;

			switch (currentState){

				default: break;
			}

		}

  
 if (animation > 0){
    fprintf(stderr, "animation = %f", animation);
    animation -= 0.05;
  }else{
	activeState = INACTIVE;
    animation = 0;

	}

}

void MTGGamePhase::CheckUserInput(float dt){
	GameObserver * game = GameObserver::GetInstance();
	if (activeState == INACTIVE){
		if (mEngine->GetButtonClick(PSP_CTRL_RTRIGGER) && game->currentActionPlayer == game->currentlyActing())
		{
			activeState = ACTIVE;
			game->userRequestNextGamePhase();
		}
	}

}
