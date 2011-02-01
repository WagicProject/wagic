#include "PrecompiledHeader.h"

#include "GameApp.h"
#include "GuiBackground.h"
#include "GameObserver.h"
#include "Rules.h"

GuiBackground::GuiBackground()
{
}

GuiBackground::~GuiBackground()
{
}

void GuiBackground::Render()
{
    JRenderer* renderer = JRenderer::GetInstance();
    JQuadPtr quad;
    GameObserver * go = GameObserver::GetInstance();
    if (go && go->mRules && go->mRules->bg.size())
    {
        quad = WResourceManager::Instance()->RetrieveTempQuad(go->mRules->bg);
    }
    if (!quad.get())
    {
        quad = WResourceManager::Instance()->RetrieveTempQuad("backdrop.jpg");
    }
    if (quad.get())
    {
        renderer->RenderQuad(quad.get(), 0, 18);
    }
}
