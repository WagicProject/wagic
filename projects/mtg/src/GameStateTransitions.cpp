#include "PrecompiledHeader.h"

#include <JRenderer.h>
#include <JGui.h>
#include "GameApp.h"
#include "GameStateTransitions.h"
#include "MTGDeck.h"
#include "Translate.h"
#include "OptionItem.h"

TransitionBase::TransitionBase(GameApp* parent, GameState* _from, GameState* _to, float duration) :
    GameState(parent)
{
    from = _from;
    to = _to;
    mDuration = duration;
    bAnimationOnly = false;
}
TransitionBase::~TransitionBase()
{
    if (!bAnimationOnly)
    {
        if (from)
            from->End();
    }
}
void TransitionBase::Update(float dt)
{
    if (from && !Finished())
        from->Update(dt);
    mElapsed += dt;
}

void TransitionBase::ButtonPressed(int controllerId, int controlId)
{
    if (!from)
        return;
    JGuiListener * jgl = dynamic_cast<JGuiListener*> (from);
    if (jgl)
        jgl->ButtonPressed(controllerId, controlId);
}

void TransitionBase::Start()
{
    mElapsed = 0;
}
;

void TransitionBase::End()
{
    mElapsed = 0;
}
;

void TransitionFade::Render()
{
    if (from)
        from->Render();
    float fade = 255 * mElapsed / mDuration;
    if (mReversed)
        fade = 255 - fade;
    JRenderer::GetInstance()->FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ARGB((int)fade,0,0,0));
}

TransitionFade::TransitionFade(GameApp* p, GameState* f, GameState* t, float dur, bool reversed) :
    TransitionBase(p, f, t, dur)
{
    mReversed = reversed;
}
;
