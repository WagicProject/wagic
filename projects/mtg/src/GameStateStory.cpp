#include "PrecompiledHeader.h"

#include "GameStateStory.h"
#include "StoryFlow.h"
#include "SimpleMenu.h"
#include "GameApp.h"
#include <dirent.h>

GameStateStory::GameStateStory(GameApp* parent) :
    GameState(parent)
{
    flow = NULL;
    menu = NULL;
}

GameStateStory::~GameStateStory()
{
    End();
}

void GameStateStory::loadStoriesMenu(const char * root)
{
    SAFE_DELETE(menu);
    stories.clear();
    DIR *mDip;
    struct dirent *mDit;

    mDip = opendir(root);

    while ((mDit = readdir(mDip)))
    {
        char buffer[4096];
        sprintf(buffer, "%s%s/story.xml", root, mDit->d_name);
        wagic::ifstream file(buffer);
        if (file)
        {
            string fname = mDit->d_name;
            stories.push_back(fname);
            file.close();
        }
    }
    closedir(mDip);

    switch (stories.size())
    {
    case 0:
        mParent->DoTransition(TRANSITION_FADE, GAME_STATE_MENU);
        break;
    case 1:
        flow = NEW StoryFlow(stories[0]);
        break;
    default:
        menu = NEW SimpleMenu(103, this, Fonts::MENU_FONT, 150, 60);
        for (size_t i = 0; i < stories.size(); ++i)
        {
            menu->Add(i, stories[i].c_str());
        }
        menu->Add(kCancelMenuID, "Cancel");
    }
}

void GameStateStory::Start()
{
    flow = NULL;
    menu = NULL;
    loadStoriesMenu(JGE_GET_RES("campaigns/").c_str());
}

void GameStateStory::Update(float dt)
{
    if (!menu && mEngine->GetButtonClick(JGE_BTN_MENU))
    {
        menu = NEW SimpleMenu(100, this, Fonts::MENU_FONT, SCREEN_WIDTH / 2 - 100, 25);
        menu->Add(0, "Back to main menu");
        menu->Add(kCancelMenuID, "Cancel");
    }
    if (menu)
    {
        menu->Update(dt);
        if (menu->isClosed())
            SAFE_DELETE(menu);
        //return;
    }
    if (flow)
    {
        if (flow->currentPageId == "End")
        {
            if (mEngine->GetButtonClick(JGE_BTN_OK) || mEngine->GetButtonClick(JGE_BTN_SEC))
            {
                mParent->DoTransition(TRANSITION_FADE, GAME_STATE_MENU);
            }
        }
        flow->Update(dt);
    }
}

void GameStateStory::Render()
{
    if (flow)
        flow->Render();
    if (menu)
        menu->Render();
}

void GameStateStory::End()
{
    SAFE_DELETE(flow);
    SAFE_DELETE(menu);
}

void GameStateStory::ButtonPressed(int controllerId, int controlId)
{
    menu->Close();

    switch (controllerId)
    {
    case 100:
        if (controlId == -1)
        {
        }
        else
        {
            mParent->DoTransition(TRANSITION_FADE, GAME_STATE_MENU);
        }
        break;
    default:
        if (controlId == -1)
        {
            mParent->DoTransition(TRANSITION_FADE, GAME_STATE_MENU);
        }
        else
        {
            flow = NEW StoryFlow(stories[controlId]);
        }
    }

}
