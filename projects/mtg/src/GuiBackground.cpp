#include "PrecompiledHeader.h"

#include "GameApp.h"
#include "GuiBackground.h"
#include "GameObserver.h"
#include "Rules.h"

const std::string kBackdropFile = "backdrop.jpg";
const std::string kBackdropFrameFile = "backdropframe.png";

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
    JQuadPtr quadframe = WResourceManager::Instance()->RetrieveTempQuad(kBackdropFrameFile);
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
        renderer->RenderQuad(quad.get(), 0, 0, 0, SCREEN_WIDTH_F / quad->mWidth, SCREEN_HEIGHT_F / quad->mHeight);
    }
    if (quadframe.get())
    {
        renderer->RenderQuad(quadframe.get(), 0, 0, 0, SCREEN_WIDTH_F / quadframe->mWidth, SCREEN_HEIGHT_F / quadframe->mHeight);
    }
}
