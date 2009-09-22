#include "../include/config.h"
#include "../include/GameApp.h"
#include "../include/GuiFrame.h"

GuiFrame::GuiFrame()
{
  if (resources.GetTexture("wood.png"))
    wood = resources.RetrieveQuad("wood.png", 0, 0, SCREEN_WIDTH, 16);
  else
    {
      wood = NULL;
      GameApp::systemError += "Can't load wood texture : " __FILE__ "\n";
    }

  if (resources.GetTexture("gold.png"))
    {
      gold1 = resources.RetrieveQuad("gold.png", 0, 0, SCREEN_WIDTH, 6, "gold1");
      gold2 = resources.RetrieveQuad("gold.png", 0, 6, SCREEN_WIDTH, 6, "gold2");
    }
  else
    {
      gold1 = gold2 = NULL;
      GameApp::systemError += "Can't load gold texture : " __FILE__ "\n";
    }
  if (resources.GetTexture("goldglow.png"))
    goldGlow = resources.RetrieveQuad("goldglow.png", 0, 1, SCREEN_WIDTH, 18);
  else
    {
      goldGlow = NULL;
      GameApp::systemError += "Can't load gold glow texture : " __FILE__ "\n";
    }

  step = 0.0;

  gold2->SetColor(ARGB(127, 255, 255, 255));
  gold2->SetHFlip(true);
}

GuiFrame::~GuiFrame()
{
  resources.Release(gold2);
  resources.Release(gold1);
  resources.Release(wood);
  resources.Release(goldGlow);
}

void GuiFrame::Render()
{
  JRenderer* renderer = JRenderer::GetInstance();
  float sized = step / 4; if (sized > SCREEN_WIDTH) sized -= SCREEN_WIDTH;
  renderer->RenderQuad(wood, 0, 0);
  renderer->RenderQuad(gold1, -sized, 16);
  renderer->RenderQuad(gold1, -sized + 479, 16);

  goldGlow->SetColor(ARGB((100+(rand()%50)), 255, 255, 255));
  renderer->RenderQuad(goldGlow, -sized, 9);
  renderer->RenderQuad(goldGlow, -sized + 480, 9);

  renderer->RenderQuad(gold2, step / 2, 16);
  renderer->RenderQuad(gold2, step / 2 - 479, 16);
}

void GuiFrame::Update(float dt)
{
  step += dt * 5;
  if (step > 2*SCREEN_WIDTH) step -= 2*SCREEN_WIDTH;
}
