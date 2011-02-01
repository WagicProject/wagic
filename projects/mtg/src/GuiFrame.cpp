#include "PrecompiledHeader.h"

#include "GameApp.h"
#include "GuiFrame.h"

GuiFrame::GuiFrame()
{
    if (WResourceManager::Instance()->GetTexture("wood.png"))
        wood = WResourceManager::Instance()->RetrieveQuad("wood.png", 0, 0, SCREEN_WIDTH, 28);
    else
    {
        GameApp::systemError += "Can't load wood texture : " __FILE__ "\n";
    }

    if (WResourceManager::Instance()->GetTexture("gold.png"))
    {
        gold1 = WResourceManager::Instance()->RetrieveQuad("gold.png", 0, 0, SCREEN_WIDTH, 6, "gold1");
        gold2 = WResourceManager::Instance()->RetrieveQuad("gold.png", 0, 6, SCREEN_WIDTH, 6, "gold2");
        if (WResourceManager::Instance()->GetTexture("goldglow.png"))
            goldGlow = WResourceManager::Instance()->RetrieveQuad("goldglow.png", 1, 1, SCREEN_WIDTH - 2, 18);
        if (gold2)
        {
            gold2->SetColor(ARGB(127, 255, 255, 255));
            gold2->SetHFlip(true);
        }
    }

    step = 0.0;

}

GuiFrame::~GuiFrame()
{
}

void GuiFrame::Render()
{
    JRenderer* renderer = JRenderer::GetInstance();
    float sized = step / 4;
    if (sized > SCREEN_WIDTH)
        sized -= SCREEN_WIDTH;
    renderer->RenderQuad(wood.get(), 0, 0);
    if (gold1.get())
    {
        renderer->RenderQuad(gold1.get(), -sized, 16);
        renderer->RenderQuad(gold1.get(), -sized + 479, 16);

        if (goldGlow.get())
        {
            goldGlow->SetColor(ARGB((100+(rand()%50)), 255, 255, 255));
            renderer->RenderQuad(goldGlow.get(), -sized, 9);
            renderer->RenderQuad(goldGlow.get(), -sized + 480, 9);
        }

        if (gold2.get())
        {
            renderer->RenderQuad(gold2.get(), step / 2, 16);
            renderer->RenderQuad(gold2.get(), step / 2 - 479, 16);
        }
    }
}

void GuiFrame::Update(float dt)
{
    step += dt * 5;
    if (step > 2 * SCREEN_WIDTH)
        step -= 2 * SCREEN_WIDTH;
}
