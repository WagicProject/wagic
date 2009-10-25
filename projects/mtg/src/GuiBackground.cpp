#include "../include/config.h"
#include "../include/GameApp.h"
#include "../include/GuiBackground.h"

GuiBackground::GuiBackground()
{
  JTexture* texture = resources.GetTexture("backdrop.jpg");
  if (texture)
    quad = NEW JQuad(texture, 0, 0, 480, 255);
  else
    {
      quad = NULL;
      GameApp::systemError = "Error loading background texture : " __FILE__;
    }
}

GuiBackground::~GuiBackground()
{
  delete(quad);
}

void GuiBackground::Render()
{
  JRenderer* renderer = JRenderer::GetInstance();
  renderer->RenderQuad(quad, 0, 18);
}
