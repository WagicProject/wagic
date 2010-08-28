#include "../include/config.h"
#include "../include/GameApp.h"
#include "../include/GuiPhaseBar.h"
#include "../include/GameObserver.h"
#include "../include/Translate.h"

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

GuiPhaseBar::GuiPhaseBar() : phase(NULL), angle(0.0f)
{
  JQuad * quad = NULL;
  if ((quad = resources.GetQuad("phasebar")) != NULL){
    quad->mHeight = Height;
    quad->mWidth = Width;
  }
  else GameApp::systemError = "Error loading phasebar texture : " __FILE__;
}

GuiPhaseBar::~GuiPhaseBar()
{
}

void GuiPhaseBar::Update(float dt)
{
  if (angle > 3*dt) angle -= 3*dt; else angle = 0;
}

void GuiPhaseBar::Render()
{
  static const float ICONSCALE = 1.5;
  static const float CENTER = SCREEN_HEIGHT_F / 2 + 10;
  JRenderer* renderer = JRenderer::GetInstance();
  GameObserver * g = GameObserver::GetInstance();
  JQuad * quad = resources.GetQuad("phasebar");
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

  //print phase name
  WFont * font = resources.GetWFont(Constants::MAIN_FONT);
  string currentP = _("your turn");
  string interrupt = "";
  if (g->currentPlayer == g->players[1]){
    currentP = _("opponent's turn");
  }
	font->SetColor(ARGB(255, 255, 255, 255));
  if (g->currentlyActing()->isAI()){
    font->SetColor(ARGB(255, 128, 128, 128));
  }
  if (g->currentlyActing() != g->currentPlayer){
    if (g->currentPlayer == g->players[0]) {
      interrupt = _(" - ")+_("opponent plays");
    }else{
      interrupt = _(" - ")+_("you play");
    }
  }

	char buf[64]; sprintf(buf, _("(%s%s) %s").c_str(), currentP.c_str(),interrupt.c_str(),_(PhaseRing::phaseName(phase->id)).c_str());
	font->DrawString(buf, SCREEN_WIDTH-5, 2,JGETEXT_RIGHT);
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
