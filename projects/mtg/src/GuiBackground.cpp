#include "../include/config.h"
#include "../include/GameApp.h"
#include "../include/GuiBackground.h"

GuiBackground::GuiBackground()
{
}

GuiBackground::~GuiBackground()
{
}

void GuiBackground::Render()
{
  JRenderer* renderer = JRenderer::GetInstance();
  renderer->RenderQuad(resources.RetrieveTempQuad("backdrop.jpg"), 0, 18);
}
