#include "PrecompiledHeader.h"

#include "GameApp.h"
#include "GuiBackground.h"
#include "GameObserver.h"
#include "Rules.h"

static std::string kBackdropFile = "";
static std::string kBackdropFrameFile = "backdropframe.png";
static std::string kPspBackdropFile = "pspbackdrop.jpg";
static std::string kPspBackdropFrameFile = "pspbackdropframe.png";

GuiBackground::GuiBackground(GameObserver* observer)
    : GuiLayer(observer)
{
}

GuiBackground::~GuiBackground()
{
    kBackdropFile = ""; //Reset the chosen backgorund.
}

void GuiBackground::Render()
{
    JRenderer* renderer = JRenderer::GetInstance();
    JQuadPtr quad;
#if !defined (PSP)
    JQuadPtr quadframe = WResourceManager::Instance()->RetrieveTempQuad(kBackdropFrameFile);
#else
    JQuadPtr quadframe = WResourceManager::Instance()->RetrieveTempQuad(kPspBackdropFrameFile);
#endif
    if (observer && observer->mRules && observer->mRules->bg.size())
    {
        quad = WResourceManager::Instance()->RetrieveTempQuad(observer->mRules->bg);
    }
    if (!quad.get())
    {
#if !defined (PSP)
        //Now it's possibile to randomly use up to 6 new background images for match (if random index is 0, it will be rendered the default "backdrop.jpg" image).
        if(kBackdropFile == ""){
            char temp[4096];
            sprintf(temp, "background/backdrop%i.jpg", std::rand() % 7);
            kBackdropFile.assign(temp);
            quad = WResourceManager::Instance()->RetrieveTempQuad(kBackdropFile);
            if (!quad.get())
                kBackdropFile = "backdrop.jpg"; //Fallback to default background image for match.
        }
        quad = WResourceManager::Instance()->RetrieveTempQuad(kBackdropFile);
#else
        quad = WResourceManager::Instance()->RetrieveTempQuad(kPspBackdropFile);
#endif
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
