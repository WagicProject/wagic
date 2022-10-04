#include "PrecompiledHeader.h"

#include "GameApp.h"
#include "GuiPhaseBar.h"
#include "GameObserver.h"
#include "Translate.h"
#include "CardSelector.h"

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

const float GuiPhaseBar::zoom_big = (float)(1.5 * 1.25);
const float GuiPhaseBar::zoom_small = 1.5;
const float GuiPhaseBar::step = M_PI/6.0f;

namespace
{
    //width and height of the phase symbol textures
    const float kWidth = 28;
    const float kHeight = kWidth;
    const unsigned kPhases = NB_MTG_PHASES - 2; //there are two phases we do not show
}

void GuiPhaseBar::DrawGlyph(JQuad *inQuad, int phaseId, float x, float y, float scale, float z)
{
    inQuad->SetTextureRect(phaseId * (kWidth + 1), z, kWidth, kHeight);
    JRenderer::GetInstance()->RenderQuad(inQuad, x, y - scale * kWidth/2, 0.0f, scale, scale);
}

GuiPhaseBar::GuiPhaseBar(DuelLayers* duelLayers) :
    GuiLayer(duelLayers->getObserver()), PlayGuiObject(80, 0, 106, 0, false),
    displayedPhaseId(0), angle(0.0f), zoomFactor(zoom_small), angleEasing(angle),
    zoomFactorEasing(zoomFactor), mpDuelLayers(duelLayers)
{
    if(duelLayers->getObserver()->getResourceManager())
    {
        JQuadPtr quad = WResourceManager::Instance()->GetQuad("phasebar");
        if (quad.get() != NULL)
        {
            quad->mHeight = kHeight;
            quad->mWidth = kWidth;
        }
        else
            GameApp::systemError = "Error loading phasebar texture : " __FILE__;
    }

    mpDuelLayers->getCardSelector()->Add(this);
}

GuiPhaseBar::~GuiPhaseBar()
{
}

void GuiPhaseBar::Update(float dt)
{
    angleEasing.update(dt);

    if(angle <= -step)
    {
        displayedPhaseId = (displayedPhaseId + 1) % kPhases;
        angleEasing.translate(step);
    }

    zoomFactorEasing.update(dt);
}

void GuiPhaseBar::Entering()
{
    mHasFocus = true;
    zoomFactorEasing.start(zoom_big, 0.3f);
}

bool GuiPhaseBar::Leaving(JButton)
{
    mHasFocus = false;
    zoomFactorEasing.start(zoom_small, 0.6f);
    return true;
}

void GuiPhaseBar::Render()
{
    JQuadPtr quad = WResourceManager::Instance()->GetQuad("phasebar");
#if defined (PSP)
    JQuadPtr phaseinfo = WResourceManager::Instance()->RetrieveTempQuad("pspfakebar.png"); //new fakebar graphics
#else
    JQuadPtr phaseinfo = WResourceManager::Instance()->RetrieveTempQuad("fakebar.png"); //new fakebar graphics
#endif
    //uncomment to draw a hideous line across hires screens.
    // JRenderer::GetInstance()->DrawLine(0, CENTER, SCREEN_WIDTH, CENTER, ARGB(255, 255, 255, 255));

    const float radius  = 25 * zoomFactor;

    for(int i = 0; i < 6; ++i)
    {
        //the position of the glyphe in the circle
        const float circPos = (i - 2) * step + angle;
        const float glyphY = this->y + this->mHeight / 2 + sin(circPos) * radius;

        //the scale is computed so that the glyphes touch each other
        //hint: sin(circPos + PI/2) = cos(circPos)
        const float glyphScale = float(zoomFactor * cosf(circPos) * 0.5f);

        if (observer->currentPlayer && observer->currentPlayer->isAI() && !observer->currentPlayer->opponent()->isAI())
            DrawGlyph(quad.get(), (displayedPhaseId - 2 + i + kPhases) % kPhases, 0, glyphY, glyphScale, 29);
        else
            DrawGlyph(quad.get(), (displayedPhaseId - 2 + i + kPhases) % kPhases, 0, glyphY, glyphScale, 0);
    }

    //print phase name
    WFont * font = WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);
    string currentP = _("your turn");
    string interrupt = "";
    if (observer->currentPlayer == mpDuelLayers->getRenderedPlayerOpponent())
    {
        currentP = _("opponent's turn");
    }
    font->SetColor(ARGB(255, 255, 255, 255));
    if (observer->currentlyActing() && observer->currentlyActing()->isAI())
    {
        font->SetColor(ARGB(255, 128, 128, 128));
    }
    if (observer->currentlyActing() != observer->currentPlayer)
    {
        if (observer->currentPlayer == mpDuelLayers->getRenderedPlayer())
        {
            interrupt = _(" - ") + _("opponent plays");
        }
        else
        {
            interrupt = _(" - ") + _("you play");
        }
    }

    char buf[200];
    //running this string through translate returns gibberish even though we defined the variables in the lang.txt
    //the conversion from phase bar phases to mtg phases is x%kPhases + 1
    //todo: just to this when the displayedPhaseId updates
    //string phaseNameToTranslate = observer->phaseRing->phaseName(displayedPhaseId%kPhases + 1);
    string phaseNameToTranslate = observer->phaseRing->phaseName(observer->phaseRing->getCurrentPhase()->id);
    phaseNameToTranslate = _(phaseNameToTranslate);
    sprintf(buf, _("(%s%s) %s").c_str(), currentP.c_str(), interrupt.c_str(),phaseNameToTranslate.c_str());
#if !defined (PSP)
    if(phaseinfo.get())
    {//fix phaseinfo graphics... should look nice now...
        float testW = ((font->GetStringWidth(buf))*2) - SCREEN_WIDTH_F;
        phaseinfo->SetHotSpot(testW+40.f, 0);
        JRenderer::GetInstance()->RenderQuad(phaseinfo.get(),0,0,0,SCREEN_WIDTH_F / phaseinfo->mWidth, SCREEN_HEIGHT_F / phaseinfo->mHeight);
    }
#endif
    font->DrawString(buf, SCREEN_WIDTH - 5, 2, JGETEXT_RIGHT);
}

int GuiPhaseBar::receiveEventMinus(WEvent *e)
{
    WEventPhaseChange *event = dynamic_cast<WEventPhaseChange*> (e);
    if (event)
    {
        //convert the mtg phase to the phases of the phase wheel
        //the mapping is
        //0  -> none
        //1..12 -> 0..11
        //13 -> none
        int targetPhase = event->to->id;

        if(targetPhase != 0 && targetPhase != 13)
        {
            targetPhase -= 1;

            int phasesToAnimate = (targetPhase - displayedPhaseId + kPhases) % kPhases;
            angleEasing.start(float(phasesToAnimate * (- step)), 0.3f * float(sqrt(float(phasesToAnimate))));
        }
    }
    return 1;
}

ostream& GuiPhaseBar::toString(ostream& out) const
{
    return out << "GuiPhaseBar";
}
