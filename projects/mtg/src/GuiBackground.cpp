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
    JQuad * quad = NULL;
    GameObserver * go = GameObserver::GetInstance();
    if (go && go->mRules && go->mRules->bg.size())
    {
        quad = resources.RetrieveTempQuad(go->mRules->bg);
    }
    if (!quad)
    {
        quad = resources.RetrieveTempQuad("backdrop.jpg");
    }
    if (!quad)
        return;
    renderer->RenderQuad(quad, 0, 18);
}
