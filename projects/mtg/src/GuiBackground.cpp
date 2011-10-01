#include "PrecompiledHeader.h"

#include "GameApp.h"
#include "GuiBackground.h"
#include "GameObserver.h"
#include "Rules.h"

const std::string kBackdropFile = "backdrop.jpg";

GuiBackground::GuiBackground(GameObserver* observer)
    : GuiLayer(observer)
{
}

GuiBackground::~GuiBackground()
{
}

void GuiBackground::Render()
{
    JRenderer* renderer = JRenderer::GetInstance();
    JQuadPtr quad;
    if (observer && observer->mRules && observer->mRules->bg.size())
    {
        quad = WResourceManager::Instance()->RetrieveTempQuad(observer->mRules->bg);
    }
    if (!quad.get())
    {
        quad = WResourceManager::Instance()->RetrieveTempQuad(kBackdropFile);
    }
    if (quad.get())
    {
        renderer->RenderQuad(quad.get(), 0, 18);
    }
}
