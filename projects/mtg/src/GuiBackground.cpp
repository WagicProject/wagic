#include "../include/config.h"
#include "../include/GameApp.h"
#include "../include/GuiBackground.h"
#include "../include/GameObserver.h"
#include "../include/Rules.h"

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
  if (go && go->mRules && go->mRules->bg.size()) {
    quad = resources.RetrieveTempQuad(go->mRules->bg);
  }
  if (!quad) {
     quad = resources.RetrieveTempQuad("backdrop.jpg");
  }
  if (!quad) return;
  renderer->RenderQuad(quad, 0, 18);
}
