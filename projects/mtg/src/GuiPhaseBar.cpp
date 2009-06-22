#include "../include/config.h"
#include "../include/GameApp.h"
#include "../include/GuiPhaseBar.h"

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

GuiPhaseBar::GuiPhaseBar(GameObserver* game) : GuiLayer(0, game), phase(GameObserver::GetInstance()->phaseRing->getCurrentPhase()), angle(0.0f)
{
  JTexture* texture = GameApp::CommonRes->GetTexture("graphics/phasebar.png");
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
  static const unsigned CENTER = SCREEN_HEIGHT / 2 - Height / 4;
  JRenderer* renderer = JRenderer::GetInstance();
  unsigned p = (phase->id + Phases - 4) * Width;
  float scale;
  float start = CENTER + (Width / 2) * angle / (M_PI / 6);

  renderer->DrawLine(0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2, ARGB(255, 255, 255, 255));

  scale = sinf(angle + 3 * M_PI / 6) / 2;
  quad->SetTextureRect((p + 3 * Width) % (Phases * Width), 0, Width, Height);
  renderer->RenderQuad(quad, 0, start, 0.0, scale, scale);
  start += Width * scale;

  scale = sinf(angle + 4 * M_PI / 6) / 2;
  quad->SetTextureRect((p + 4 * Width) % (Phases * Width), Height, Width, Height);
  renderer->RenderQuad(quad, 0, start, 0.0, scale, scale);
  start += Width * scale;

  scale = sinf(angle + 5 * M_PI / 6) / 2;
  quad->SetTextureRect((p + 5 * Width) % (Phases * Width), Height, Width, Height);
  renderer->RenderQuad(quad, 0, start, 0.0, scale, scale);
  start += Width * scale;

  start = CENTER + (Width / 2) * angle / (M_PI / 6);

  scale = sinf(angle + 2 * M_PI / 6) / 2;
  start -= Width * scale;
  quad->SetTextureRect((p + 2 * Width) % (Phases * Width), Height, Width, Height);
  renderer->RenderQuad(quad, 0, start, 0.0, scale, scale);

  scale = sinf(angle + 1 * M_PI / 6) / 2;
  start -= Width * scale;
  quad->SetTextureRect((p + 1 * Width) % (Phases * Width), Height, Width, Height);
  renderer->RenderQuad(quad, 0, start, 0.0, scale, scale);

  if (angle > 0)
    {
      scale = sinf(angle)/2;
      start -= Width * scale;
      quad->SetTextureRect(p % (Phases * Width), Height, Width, Height);
      renderer->RenderQuad(quad, 0, start, 0.0, scale, scale);
    }
}

int GuiPhaseBar::receiveEvent(WEvent *e)
{
  WEventPhaseChange *event = dynamic_cast<WEventPhaseChange*>(e);
  if (event)
    {
      angle = M_PI / 6;
      phase = event->to;
    }
  return 1;
}
