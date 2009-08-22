#include "../include/config.h"
#include "../include/GameApp.h"
#include "../include/GuiFrame.h"

GuiFrame::GuiFrame()
{
  if (JTexture* woodTex = GameApp::CommonRes->GetTexture("graphics/wood.png"))
    wood = NEW JQuad(woodTex, 0, 0, SCREEN_WIDTH, 16);
  else
    {
      wood = NULL;
      GameApp::systemError += "Can't load wood texture : " __FILE__ "\n";
    }

  if (JTexture* goldTex = GameApp::CommonRes->GetTexture("graphics/gold.png"))
    {
      gold1 = NEW JQuad(goldTex, 0, 0, SCREEN_WIDTH, 6);
      gold2 = NEW JQuad(goldTex, 0, 6, SCREEN_WIDTH, 6);
    }
  else
    {
      gold1 = gold2 = NULL;
      GameApp::systemError += "Can't load gold texture : " __FILE__ "\n";
    }
  if (JTexture* goldGlowTex = GameApp::CommonRes->GetTexture("graphics/goldglow.png"))
    goldGlow = NEW JQuad(goldGlowTex, 0, 1, SCREEN_WIDTH, 18);
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
  delete(gold2);
  delete(gold1);
  delete(wood);
}

void GuiFrame::Render()
{
  JRenderer* renderer = JRenderer::GetInstance();
  float sized = step; if (sized > SCREEN_WIDTH) sized -= SCREEN_WIDTH;
  renderer->RenderQuad(wood, 0, 0);
  renderer->RenderQuad(gold1, -sized, 16);
  renderer->RenderQuad(gold1, -sized + 479, 16);

  goldGlow->SetColor(ARGB(100+(random()%50), 255, 255, 255));
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
