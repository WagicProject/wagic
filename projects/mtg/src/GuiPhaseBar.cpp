#include "../include/config.h"
#include "../include/GameApp.h"
#include "../include/GuiPhaseBar.h"

/*
static int colors[] =
  {
    ARGB(255, 255, 255, 255),
    ARGB(255, 255, 000, 000),
    ARGB(255, 000, 255, 000),
    ARGB(255, 000, 000, 255),
    ARGB(255, 255, 255, 000),
    ARGB(255, 255, 000, 255),
    ARGB(255, 000, 255, 255),
    ARGB(255, 000, 000, 000),
    ARGB(255, 255, 255, 255),
    ARGB(255, 255, 255, 255),
    ARGB(255, 255, 255, 255),
    ARGB(255, 255, 255, 255)
  };
*/

GuiPhaseBar::GuiPhaseBar() : phase(GameObserver::GetInstance()->phaseRing->getCurrentPhase()), angle(0.0f)
{
  JTexture* texture = GameApp::CommonRes->GetTexture("phasebar.png");
  if (texture)
    quad = NEW JQuad(texture, 0, 0, Width, Height);
  else
    {
      quad = NULL;
      GameApp::systemError = "Error loading phasebar texture : " __FILE__;
    }
}

GuiPhaseBar::~GuiPhaseBar()
{
  delete(quad);
}

void GuiPhaseBar::Update(float dt)
{
  if (angle > 0) angle -= 3*dt; else angle = 0;
}

void GuiPhaseBar::Render()
{
  static const float ICONSCALE = 1.5;
  static const unsigned CENTER = SCREEN_HEIGHT / 2 + 10;
  JRenderer* renderer = JRenderer::GetInstance();
  unsigned p = (phase->id + Phases - 4) * (Width+1);
  float scale;
  float start = CENTER + (Width / 2) * angle * ICONSCALE / (M_PI / 6) - ICONSCALE * Width / 4;

  renderer->DrawLine(0, CENTER, SCREEN_WIDTH, CENTER, ARGB(255, 255, 255, 255));

  scale = ICONSCALE * sinf(angle + 3 * M_PI / 6) / 2;
  quad->SetTextureRect((p + 3 * (Width+1)) % (Phases * (Width+1)), 0, Width, Height);
  renderer->RenderQuad(quad, 0, start, 0.0, scale, scale);
  start += Width * scale;

  scale = ICONSCALE * sinf(angle + 4 * M_PI / 6) / 2;
  quad->SetTextureRect((p + 4 * (Width+1)) % (Phases * (Width+1)), Height, Width, Height);
  renderer->RenderQuad(quad, 0, start, 0.0, scale, scale);
  start += Width * scale;

  scale = ICONSCALE * sinf(angle + 5 * M_PI / 6) / 2;
  quad->SetTextureRect((p + 5 * (Width+1)) % (Phases * (Width+1)), Height, Width, Height);
  renderer->RenderQuad(quad, 0, start, 0.0, scale, scale);
  start += Width * scale;

  start = CENTER + (Width / 2) * angle * ICONSCALE / (M_PI / 6) - ICONSCALE * Width / 4;

  scale = ICONSCALE * sinf(angle + 2 * M_PI / 6) / 2;
  start -= Width * scale;
  quad->SetTextureRect((p + 2 * (Width+1)) % (Phases * (Width+1)), Height, Width, Height);
  renderer->RenderQuad(quad, 0, start, 0.0, scale, scale);

  scale = ICONSCALE * sinf(angle + 1 * M_PI / 6) / 2;
  start -= Width * scale;
  quad->SetTextureRect((p + 1 * (Width+1)) % (Phases * (Width+1)), Height, Width, Height);
  renderer->RenderQuad(quad, 0, start, 0.0, scale, scale);

  if (angle > 0)
    {
      scale = ICONSCALE * sinf(angle)/2;
      start -= Width * scale;
      quad->SetTextureRect(p % (Phases * (Width+1)), Height, Width, Height);
      renderer->RenderQuad(quad, 0, start, 0.0, scale, scale);
    }
}

int GuiPhaseBar::receiveEventMinus(WEvent *e)
{
  WEventPhaseChange *event = dynamic_cast<WEventPhaseChange*>(e);
  if (event)
    {
      angle = M_PI / 6;
      phase = event->to;
    }
  return 1;
}
