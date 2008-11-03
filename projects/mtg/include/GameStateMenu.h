#ifndef _GAME_STATE_MENU_H_
#define _GAME_STATE_MENU_H_

#include <JGui.h>
#include <dirent.h>

#include "GameState.h"
#include "MenuItem.h"
#include "SimpleMenu.h"

#include "../include/GameOptions.h"



#define STATE_MENU			0
#define STATE_SUBMENU			1
#define STATE_LOADING_MENU 2
#define STATE_LOADING_CARDS 3
#define STATE_FIRST_TIME 4
#define STATE_WARNING 5

#define GAME_VERSION "WTH?! 0.2.2 - by WilLoW"
#define ALPHA_WARNING 0

class GameStateMenu:	public GameState, public JGuiListener

{
private:
	JGuiController* mGuiController;
	SimpleMenu* subMenuController;
	JLBFont* mFont;
	JQuad * mIcons[10];
	JTexture * mIconsTexture;
	JTexture * bgTexture;
	JQuad * mBg;
	float mCreditsYPos;
	int currentState;
	JMusic * bgMusic;
	int mVolume;
	char nbcardsStr[400];

	DIR *mDip;
	struct dirent *mDit;
	char mCurrentSetName[10];
	char mCurrentSetFileName[512];

	int mReadConf;

	
public:
	GameStateMenu(GameApp* parent): GameState(parent) 
	{
		mGuiController = NULL;
		subMenuController = NULL;
		mIconsTexture = NULL;
		bgMusic = NULL;
	}

	virtual ~GameStateMenu() 
	{

	}


	virtual void Create()
	{
		
		mDip = NULL;
		mReadConf = 0;
		mCurrentSetName[0] = 0;





	mIconsTexture = JRenderer::GetInstance()->LoadTexture("graphics/menuicons.png", TEX_TYPE_USE_VRAM);
	bgTexture = JRenderer::GetInstance()->LoadTexture("graphics/menutitle.png", TEX_TYPE_USE_VRAM);	
	mBg = NEW JQuad(bgTexture, 10, 0, 220, 80);		// Create background quad for rendering.
	mBg->SetHotSpot(105,32);
	 //load all the icon images
	int n = 0;
	for (int i=0;i<5;i++)
	{
		for (int j=0;j<2;j++)
		{
			mIcons[n] = NEW JQuad(mIconsTexture, 10 + i*32, j*32, 32, 32);
			mIcons[n]->SetHotSpot(16,16);
			n++;
		}
	}



	   mFont = GameApp::CommonRes->GetJLBFont("graphics/f3");
		//mFont->SetBase(0);	// using 2nd font
		mGuiController = NEW JGuiController(100, this);
		//mGuiController->SetShadingBackground(10, 45, 80, 100, ARGB(255,0,0,0));
		if (mGuiController)
		{	
			mGuiController->Add(NEW MenuItem(1, mFont, "Play", 80, SCREEN_HEIGHT/2, mIcons[8], mIcons[9],"graphics/particle1.psi",GameApp::CommonRes->GetQuad("particles"),  true));
			mGuiController->Add(NEW MenuItem(2, mFont, "Deck Editor", 160, SCREEN_HEIGHT/2, mIcons[2], mIcons[3],"graphics/particle2.psi",GameApp::CommonRes->GetQuad("particles")));
			mGuiController->Add(NEW MenuItem(3, mFont, "Shop", 240, SCREEN_HEIGHT/2, mIcons[0], mIcons[1],"graphics/particle3.psi",GameApp::CommonRes->GetQuad("particles")));
			mGuiController->Add(NEW MenuItem(4, mFont, "Options", 320, SCREEN_HEIGHT/2, mIcons[6], mIcons[7],"graphics/particle4.psi",GameApp::CommonRes->GetQuad("particles")));
			mGuiController->Add(NEW MenuItem(5, mFont, "Exit", 400, SCREEN_HEIGHT/2, mIcons[4], mIcons[5],"graphics/particle5.psi",GameApp::CommonRes->GetQuad("particles")));
		}


	currentState = STATE_LOADING_CARDS;





	}


	virtual void Destroy()
	{
		if (mGuiController)
			delete mGuiController;

		if (subMenuController)
			delete subMenuController;

		if (mIconsTexture)
				delete mIconsTexture;

		for (int i = 0; i < 10 ; i++){
			delete mIcons[i];
		}

		if (mBg) delete mBg;

				  SAFE_DELETE (bgMusic);
	}

	
	virtual void Start(){
	  JRenderer::GetInstance()->ResetPrivateVRAM();
 JRenderer::GetInstance()->EnableVSync(true);

			if (GameApp::HasMusic && !bgMusic && GameOptions::GetInstance()->values[OPTIONS_MUSICVOLUME] > 0){
				bgMusic = JSoundSystem::GetInstance()->LoadMusic("sound/track0.mp3");
			}

							if (bgMusic){
					mVolume = 0;
					JSoundSystem::GetInstance()->SetVolume(mVolume);
					JSoundSystem::GetInstance()->PlayMusic(bgMusic, true);
				}

	}


	/* Retrieves the next set subfolder automatically
	*/
	int nextCardSet(){
		int found = 0;
		if (!mDip){
			 mDip = opendir("Res/sets/");
		}

		while (!found && (mDit = readdir(mDip))){
			sprintf(mCurrentSetFileName, "Res/sets/%s/_cards.dat", mDit->d_name);
			std::ifstream file(mCurrentSetFileName);
			if(file){
				sprintf(mCurrentSetName, "%s", mDit->d_name);
				file.close();
				found = 1;
			}
		}
		if (!mDit) {
			closedir(mDip);
			mDip = NULL;
		}
		return found;
	}

	virtual void End()
	{
	  //mEngine->EnableVSync(false);
		
		if (bgMusic)
		{
			JSoundSystem::GetInstance()->StopMusic(bgMusic);
		}
		JRenderer::GetInstance()->EnableVSync(false);
	}


	virtual void Update(float dt)
	{
		 if (bgMusic && mVolume < 2*GameOptions::GetInstance()->values[OPTIONS_MUSICVOLUME]){
				mVolume++;
				JSoundSystem::GetInstance()->SetVolume(mVolume/2);
			}

		if (currentState == STATE_LOADING_CARDS){
			if (mReadConf){
					mParent->collection->load(mCurrentSetFileName, mCurrentSetName);
			}else{
				mReadConf = 1;
			}
			if (!nextCardSet()){
				//How many cards total ?
				sprintf(nbcardsStr, "Database: %i cards", mParent->collection->totalCards());
				//Check for first time comer
				std::ifstream file("Res/player/collection.dat");
				if(file){
					file.close();
					currentState = STATE_WARNING;
				}else{
					currentState = STATE_FIRST_TIME;
				}
			}
		}else if (currentState == STATE_FIRST_TIME){
			//Give the player cards from the set for which we have the most variety
			int setId = 0;
			int maxcards = 0;
			for (int i=0; i< MtgSets::SetsList->nb_items; i++){
				int value = mParent->collection->countBySet(i);
				if (value > maxcards){
					maxcards = value;
					setId = i;
				}
			}
			createUsersFirstDeck(setId);
			currentState = STATE_WARNING;
		}else if (currentState == STATE_WARNING){
			if (!ALPHA_WARNING){
				currentState = STATE_MENU;
			}else{
					if (mEngine->GetButtonClick(PSP_CTRL_CIRCLE)) currentState = STATE_MENU; 
			}
		}else{
			if (currentState == STATE_MENU && mGuiController!=NULL)
				mGuiController->Update(dt);
			if (currentState == STATE_SUBMENU){
				if( subMenuController != NULL){
					subMenuController->Update(dt);
				}else{
						subMenuController = NEW SimpleMenu(102, this,mFont, 50,170,SCREEN_WIDTH-120);
						if (subMenuController){	
							subMenuController->Add(11,"1 Player");
							subMenuController->Add(12, "2 Players");
							subMenuController->Add(13,"Demo");
							subMenuController->Add(14, "Cancel");
							#ifdef TESTSUITE
								subMenuController->Add(666, "Test Suite");
							#endif
						}
				}
			}
		}
		if (currentState == STATE_WARNING && !ALPHA_WARNING) currentState = STATE_MENU;
	}

	void createUsersFirstDeck(int setId){
#if defined (WIN32) || defined (LINUX)
		char buf[4096];
		sprintf(buf, "setID: %i", setId);
		OutputDebugString(buf);
#endif
		MTGDeck *mCollection = NEW MTGDeck("Res/player/collection.dat", mParent->cache, mParent->collection);
		//10 lands of each
		if (!mCollection->addRandomCards(10, setId,RARITY_L,"Forest")){
			mCollection->addRandomCards(10, -1,RARITY_L,"Forest");
		}
		if (!mCollection->addRandomCards(10, setId,RARITY_L,"Plains")){
			mCollection->addRandomCards(10, -1,RARITY_L,"Plains");
		}
		if (!mCollection->addRandomCards(10, setId,RARITY_L,"Swamp")){
			mCollection->addRandomCards(10, -1,RARITY_L,"Swamp");
		}
		if (!mCollection->addRandomCards(10, setId,RARITY_L,"Mountain")){
			mCollection->addRandomCards(10, -1,RARITY_L,"Mountain");
		}
		if (!mCollection->addRandomCards(10, setId,RARITY_L,"Island")){
			mCollection->addRandomCards(10, -1,RARITY_L,"Island");
		}


#if defined (WIN32) || defined (LINUX)
		OutputDebugString("1\n");
#endif

		//Starter Deck
		mCollection->addRandomCards(3, setId,RARITY_R,NULL);
		mCollection->addRandomCards(9, setId,RARITY_U,NULL);
		mCollection->addRandomCards(48, setId,RARITY_C,NULL);

#if defined (WIN32) || defined (LINUX)
		OutputDebugString("2\n");
#endif
		//Boosters
		for (int i = 0; i< 2; i++){
			mCollection->addRandomCards(1, setId,RARITY_R);
			mCollection->addRandomCards(3, setId,RARITY_U);
			mCollection->addRandomCards(11, setId,RARITY_C);
		}
		mCollection->save();
		delete mCollection;
	}

	virtual void Render() 
	{

		JRenderer * renderer = JRenderer::GetInstance();
		renderer->ClearScreen(ARGB(0,0,0,0));

		if (currentState == STATE_LOADING_CARDS){
			char text[512];
			sprintf(text, "LOADING SET: %s", mCurrentSetName);
			mFont->DrawString(text,SCREEN_WIDTH/2,SCREEN_HEIGHT/2,JGETEXT_CENTER);
		}else{


			PIXEL_TYPE colors[] =
				{
					ARGB(255,17,17,17),
					ARGB(255,17,17,17),
					ARGB(255,62,62,62),
					ARGB(255,62,62,62)
				};



			renderer->FillRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,colors);
			renderer->RenderQuad(mBg, SCREEN_WIDTH/2  , 50);
				if (mGuiController!=NULL)
					mGuiController->Render();

			mFont->SetScale(0.7);
			mFont->SetColor(ARGB(128,255,255,255));
			mFont->DrawString(GAME_VERSION, SCREEN_WIDTH-10,SCREEN_HEIGHT-15,JGETEXT_RIGHT);
			mFont->DrawString(nbcardsStr,10, SCREEN_HEIGHT-15);
			mFont->SetScale(1.f);
			mFont->SetColor(ARGB(255,255,255,255));
			if (currentState == STATE_SUBMENU && subMenuController != NULL){
				subMenuController->Render();
			}

			if (currentState == STATE_WARNING){
				renderer->FillRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,ARGB(128,0,0,0));

				mFont->DrawString("IMPORTANT NOTE" ,SCREEN_WIDTH/2,10,JGETEXT_CENTER);
				mFont->DrawString("This is an alpha version with lots of bugs.",SCREEN_WIDTH/2,35,JGETEXT_CENTER);
				mFont->DrawString("It WILL crash your psp" ,SCREEN_WIDTH/2,50,JGETEXT_CENTER);
				mFont->DrawString("If you use it anyway, your feedback is welcome" ,SCREEN_WIDTH/2,65,JGETEXT_CENTER);

				mFont->DrawString("This freeware game is NOT published or endorsed" ,SCREEN_WIDTH/2,110,JGETEXT_CENTER);
			mFont->DrawString("by Wizard of the Coast, Inc." ,SCREEN_WIDTH/2,125,JGETEXT_CENTER);
				mFont->DrawString("Infos & updates at http://wololo.net/wagic/" ,SCREEN_WIDTH/2,170,JGETEXT_CENTER);			
				mFont->DrawString("PRESS CIRCLE TO CONTINUE OR HOME TO QUIT" ,SCREEN_WIDTH/2,210,JGETEXT_CENTER);

			}



		}
}

	
	virtual void ButtonPressed(int controllerId, int controlId)
	{
#if defined (WIN32) || defined (LINUX)
		char buf[4096];
		sprintf(buf, "cnotrollerId: %i", controllerId);
		OutputDebugString(buf);
#endif
		switch (controllerId){
			case 101:
				createUsersFirstDeck(controlId);
				currentState = STATE_MENU;
				SAFE_DELETE(subMenuController);
				break;
			default:
				switch (controlId)
				{
				case 1:
					currentState = STATE_SUBMENU;
					break;
				case 2:		
					mParent->SetNextState(GAME_STATE_DECK_VIEWER);
					break;
				case 3:
					mParent->SetNextState(GAME_STATE_SHOP);
					break;
				case 4:
					mParent->SetNextState(GAME_STATE_OPTIONS);
					break;
				case 5:
					mEngine->End();
					break;
				case 11:
					mParent->players[0] = PLAYER_TYPE_HUMAN;
					mParent->players[1] = PLAYER_TYPE_CPU;
					mParent->SetNextState(GAME_STATE_DUEL);
					break;
				case 12:
					mParent->players[0] = PLAYER_TYPE_HUMAN;
					mParent->players[1] = PLAYER_TYPE_HUMAN;
					mParent->SetNextState(GAME_STATE_DUEL);
					break;
				case 13:
					mParent->players[0] = PLAYER_TYPE_CPU;
					mParent->players[1] = PLAYER_TYPE_CPU;
					mParent->SetNextState(GAME_STATE_DUEL);
					break;
				case 14:
					currentState = STATE_MENU;
					delete subMenuController;
					subMenuController = NULL;
					break;
#ifdef TESTSUITE
				case 666:
					mParent->players[0] = PLAYER_TYPE_TESTSUITE;
					mParent->players[1] = PLAYER_TYPE_TESTSUITE;
					mParent->SetNextState(GAME_STATE_DUEL);
					break;
#endif
				}
			break;
		}
	}
		

};


#endif

