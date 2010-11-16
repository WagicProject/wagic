#include "PrecompiledHeader.h"

#include "GameApp.h"
#include "GuiFrame.h"

GuiFrame::GuiFrame()
{
    if (resources.GetTexture("wood.png"))
        wood = resources.RetrieveQuad("wood.png", 0, 0, SCREEN_WIDTH, 28);
    else
    {
        wood = NULL;
        GameApp::systemError += "Can't load wood texture : " __FILE__ "\n";
    }

    goldGlow = gold1 = gold2 = NULL;
    if (resources.GetTexture("gold.png"))
    {
        gold1 = resources.RetrieveQuad("gold.png", 0, 0, SCREEN_WIDTH, 6, "gold1");
        gold2 = resources.RetrieveQuad("gold.png", 0, 6, SCREEN_WIDTH, 6, "gold2");
        if (resources.GetTexture("goldglow.png"))
            goldGlow = resources.RetrieveQuad("goldglow.png", 1, 1, SCREEN_WIDTH - 2, 18);
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
    renderer->RenderQuad(wood, 0, 0);
    if (gold1)
    {
        renderer->RenderQuad(gold1, -sized, 16);
        renderer->RenderQuad(gold1, -sized + 479, 16);

        if (goldGlow)
        {
            goldGlow->SetColor(ARGB((100+(rand()%50)), 255, 255, 255));
            renderer->RenderQuad(goldGlow, -sized, 9);
            renderer->RenderQuad(goldGlow, -sized + 480, 9);
        }

        if (gold2)
        {
            renderer->RenderQuad(gold2, step / 2, 16);
            renderer->RenderQuad(gold2, step / 2 - 479, 16);
        }
    }
}

void GuiFrame::Update(float dt)
{
    step += dt * 5;
    if (step > 2 * SCREEN_WIDTH)
        step -= 2 * SCREEN_WIDTH;
}
