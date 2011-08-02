#include "PrecompiledHeader.h"

#include "GameApp.h"
#include "GuiPhaseBar.h"
#include "GameObserver.h"
#include "Translate.h"

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

namespace
{
    const float kWidth = 28;
    const float kHeight = kWidth;
    const unsigned kPhases = 12;

    const float ICONSCALE = 1.5;
    const float CENTER = SCREEN_HEIGHT_F / 2 + 10;

    void DrawGlyph(JQuad* inQuad, int inGlyph, float inY, float inAngle, unsigned int inP, float inScale)
    {
        float xPos = static_cast<float> ((inP + inGlyph * (int) (kWidth + 1)) % (kPhases * (int) (kWidth + 1)));
        inQuad->SetTextureRect(xPos, 0, kWidth, kHeight);
        JRenderer::GetInstance()->RenderQuad(inQuad, 0, inY, 0.0, inScale, inScale);
    }
}

GuiPhaseBar* GuiPhaseBar::instance = NULL;


GuiPhaseBar* GuiPhaseBar::GetInstance()
{
  return GuiPhaseBar::instance;
}

void GuiPhaseBar::Zoom(float zoom)
{
  zoomTarget = zoom*ICONSCALE;
}

GuiPhaseBar::GuiPhaseBar() :
  phase(NULL), angle(0.0f), zoomTarget(ICONSCALE), zoomFactor(ICONSCALE)
{
    JQuadPtr quad = WResourceManager::Instance()->GetQuad("phasebar");
    if (quad.get() != NULL)
    {
        quad->mHeight = kHeight;
        quad->mWidth = kWidth;
    }
    else
        GameApp::systemError = "Error loading phasebar texture : " __FILE__;

    instance = this;
}

GuiPhaseBar::~GuiPhaseBar()
{
}

void GuiPhaseBar::Update(float dt)
{
    if (angle > 3 * dt)
        angle -= 3 * dt;
    else
        angle = 0;

    if(zoomFactor + 0.05 < zoomTarget)
    {
      zoomFactor += (float)0.05;
    }
    else if (zoomFactor - 0.05 > zoomTarget)
    {
      zoomFactor -= (float)0.05;
    }
}

void GuiPhaseBar::Render()
{
    GameObserver * g = GameObserver::GetInstance();
    JQuadPtr quad = WResourceManager::Instance()->GetQuad("phasebar");

    JRenderer::GetInstance()->DrawLine(0, CENTER, SCREEN_WIDTH, CENTER, ARGB(255, 255, 255, 255));

    unsigned int p = (phase->id + kPhases - 4) * (int) (kWidth + 1);
    float centerYPosition = CENTER + (kWidth / 2) * angle * zoomFactor / (M_PI / 6) - zoomFactor * kWidth / 4;
    float yPos = centerYPosition;
    float scale = 0;
    for (int glyph = 3; glyph < 6; ++glyph)
    {
        scale = zoomFactor * sinf(angle + glyph * M_PI / 6) / 2;
        DrawGlyph(quad.get(), glyph, yPos, angle, p, scale);
        yPos += kWidth * scale;
    }

    yPos = centerYPosition;
    for (int glyph = 2; glyph > 0; --glyph)
    {
        scale = zoomFactor * sinf(angle + glyph * M_PI / 6) / 2;
        yPos -= kWidth * scale;
        DrawGlyph(quad.get(), glyph, yPos, angle, p, scale);
    }

    if (angle > 0)
    {
        scale = zoomFactor * sinf(angle) / 2;
        yPos -= kWidth * scale;
        float xPos = static_cast<float> (p % (kPhases * (int) (kWidth + 1)));
        quad->SetTextureRect(xPos, kHeight, kWidth, kHeight);
        JRenderer::GetInstance()->RenderQuad(quad.get(), 0, yPos, 0.0, scale, scale);
    }

    //print phase name
    WFont * font = WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);
    string currentP = _("your turn");
    string interrupt = "";
    if (g->currentPlayer == g->players[1])
    {
        currentP = _("opponent's turn");
    }
    font->SetColor(ARGB(255, 255, 255, 255));
    if (g->currentlyActing()->isAI())
    {
        font->SetColor(ARGB(255, 128, 128, 128));
    }
    if (g->currentlyActing() != g->currentPlayer)
    {
        if (g->currentPlayer == g->players[0])
        {
            interrupt = _(" - ") + _("opponent plays");
        }
        else
        {
            interrupt = _(" - ") + _("you play");
        }
    }

    char buf[64];
    sprintf(buf, _("(%s%s) %s").c_str(), currentP.c_str(), interrupt.c_str(), _(PhaseRing::phaseName(phase->id)).c_str());
    font->DrawString(buf, SCREEN_WIDTH - 5, 2, JGETEXT_RIGHT);
}

int GuiPhaseBar::receiveEventMinus(WEvent *e)
{
    WEventPhaseChange *event = dynamic_cast<WEventPhaseChange*> (e);
    if (event)
    {
        angle = M_PI / 6;
        phase = event->to;
    }
    return 1;
}
