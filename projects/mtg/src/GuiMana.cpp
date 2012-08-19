#include "PrecompiledHeader.h"

#include "GuiMana.h"
#include "OptionItem.h"
#include "Player.h"

//using std::cout;
using std::endl;

ManaIcon::ManaIcon(int color, float x, float y, float destx, float desty) :
    Pos(x, y, 0.5, 0.0, 255), f(-1), destx(destx), desty(desty), mode(ALIVE), color(color)
{
    hgeParticleSystemInfo * psi = NULL;
    JQuadPtr mq = WResourceManager::Instance()->GetQuad("stars");

    if (!mq.get())
    {
        particleSys = NULL;
        return;
    }

    switch (color)
    {
    case Constants::MTG_COLOR_RED:
        psi = WResourceManager::Instance()->RetrievePSI("manared.psi", mq.get());
        break;
    case Constants::MTG_COLOR_BLUE:
        psi = WResourceManager::Instance()->RetrievePSI("manablue.psi", mq.get());
        break;
    case Constants::MTG_COLOR_GREEN:
        psi = WResourceManager::Instance()->RetrievePSI("managreen.psi", mq.get());
        break;
    case Constants::MTG_COLOR_BLACK:
        psi = WResourceManager::Instance()->RetrievePSI("manablack.psi", mq.get());
        break;
    case Constants::MTG_COLOR_WHITE:
        psi = WResourceManager::Instance()->RetrievePSI("manawhite.psi", mq.get());
        break;
    default:
        psi = WResourceManager::Instance()->RetrievePSI("mana.psi", mq.get());
    }

    if (!psi)
    {
        psi = NEW hgeParticleSystemInfo();
        if (!psi)
            return;
        hgeParticleSystemInfo * defaults = WResourceManager::Instance()->RetrievePSI("mana.psi", mq.get());
        if (defaults)
        {
            memcpy(psi, defaults, sizeof(hgeParticleSystemInfo));
        }
        else
        {
            memset(psi, 0, sizeof(hgeParticleSystemInfo));

            //Default values for particle system! Cribbed from mana.psi
            //Really, we should just be loading that and then changing colors...
            psi->nEmission = 114;
            psi->fLifetime = -1;
            psi->fParticleLifeMin = 1.1507937f;
            psi->fParticleLifeMax = 1.4682540f;
            psi->fSpeedMin = 0.0099999998f;
            psi->fSizeStart = 0.5f;
            psi->fSizeEnd = 0.69999999f;
            psi->fSizeVar = 0.25396827f;
            psi->fSpinStart = -5.5555553f;
            psi->fAlphaVar = 0.77777779f;
            psi->sprite = mq.get();
        }

        switch (color)
        {
        case Constants::MTG_COLOR_RED:
            psi->colColorStart.SetHWColor(ARGB(161,240,40,44));
            psi->colColorEnd.SetHWColor(ARGB(14,242,155,153));
            break;
        case Constants::MTG_COLOR_BLUE:
            psi->colColorStart.SetHWColor(ARGB(161,28,40,224));
            psi->colColorEnd.SetHWColor(ARGB(14,255,255,255));
            break;
        case Constants::MTG_COLOR_GREEN:
            psi->colColorStart.SetHWColor(ARGB(161,36,242,44));
            psi->colColorEnd.SetHWColor(ARGB(14,129,244,153));
            break;
        case Constants::MTG_COLOR_BLACK:
            psi->colColorStart.SetHWColor(ARGB(161,210,117,210));
            psi->colColorEnd.SetHWColor(ARGB(14,80,56,80));
            break;
        case Constants::MTG_COLOR_WHITE:
            psi->colColorStart.SetHWColor(ARGB(151,151,127,38));
            psi->colColorEnd.SetHWColor(ARGB(8,255,255,255));
            break;
        default:
            psi->colColorStart.SetHWColor(ARGB(161,236,242,232));
            psi->colColorEnd.SetHWColor(ARGB(14,238,244,204));
            break;
        }

        particleSys = NEW hgeParticleSystem(psi);
        SAFE_DELETE(psi); //This version of psi is not handled by cache, so kill it here.
    }
    else
        particleSys = NEW hgeParticleSystem(psi); //Cache will clean psi up later.

    // if we want to throttle the amount of particles for mana,
    // here's where to do it - this is hardcoded to something like 114 in the psi file
		if(OptionManaDisplay::NOSTARSDYNAMIC == options[Options::MANADISPLAY].number)
		{
    particleSys->info.nEmission = 0;
		}
		else
		{
    particleSys->info.nEmission = 60;
		}
    icon = manaIcons[color];

    particleSys->FireAt(x, y);

    zoomP1 = 0.2f + 0.1f * ((float) rand() / (float) RAND_MAX);
    zoomP2 = 0.2f + 0.1f * ((float) rand() / (float) RAND_MAX);
    zoomP3 = 2 * M_PI * ((float) rand() / (float) RAND_MAX);
    zoomP4 = 2 * M_PI * ((float) rand() / (float) RAND_MAX);
    zoomP5 = 0.5f + ((float) rand() / (float) RAND_MAX);
    zoomP6 = 0.5f + ((float) rand() / (float) RAND_MAX);

    xP1 = 2 * M_PI * ((float) rand() / (float) RAND_MAX);
    xP2 = 5 + 30 * ((float) rand() / (float) RAND_MAX);
    xP3 = 0.5f + ((float) rand() / (float) RAND_MAX);
    yP1 = 2 * M_PI * ((float) rand() / (float) RAND_MAX);
    yP2 = 5 + 10 * ((float) rand() / (float) RAND_MAX);
    yP3 = 0.5f + ((float) rand() / (float) RAND_MAX);

    actT = 0;
    tP1 = 0;
}

ManaIcon::~ManaIcon()
{
    SAFE_DELETE(particleSys);
}

void ManaIcon::Render()
{
    if (!particleSys)
        return;

    JRenderer* renderer = JRenderer::GetInstance();

    renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
    particleSys->Render();
    renderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
    renderer->RenderQuad(icon.get(), actX, actY, actT, actZ + zoomP1 * sinf(M_PI * zoomP3), actZ + zoomP2 * cosf(M_PI * zoomP4));
}
void ManaIcon::Update(float dt, float shift)
{
    xP1 += xP3 * dt;
    actX = x + xP2 * sinf(M_PI * xP1);
    zoomP3 += zoomP5 * dt;
    zoomP4 += zoomP6 * dt;

    if (OptionManaDisplay::STATIC == options[Options::MANADISPLAY].number)
        shift = 0;

    switch (mode)
    {
    case DROPPING:
        f += dt * 700;
        actY += f * dt;
        if (actY > SCREEN_HEIGHT * 2)
            mode = DEAD;
        break;
    case WITHERING:
        actT += dt * 4;
        actZ /= f;
        zoomP1 /= f;
        zoomP2 /= f;
        f -= dt;
        actZ *= f;
        zoomP1 *= f;
        zoomP2 *= f;
        yP1 += yP3 * dt;
        actY = y + yP2 * sinf(M_PI * yP1);
        if (f < 0)
            mode = DEAD;
        break;
    case ALIVE:
        x += 10 * dt * (destx - x);
        y += 10 * dt * (desty + shift - y);
        yP1 += yP3 * dt;
        actY = y + yP2 * sinf(M_PI * yP1);

        if (particleSys && (fabs(destx - x) < 5) && (fabs(desty + shift - y) < 5))
        {
					if (OptionManaDisplay::STATIC == options[Options::MANADISPLAY].number)
            {
                SAFE_DELETE(particleSys); //Static Mana Only: avoid expensive particle processing
            }
        }
        break;
    case DEAD:
        break;
    }

    if (particleSys)
    {
        particleSys->MoveTo(actX, actY);
        particleSys->Update(dt);
    }
}

void ManaIcon::Wither()
{
    mode = WITHERING;
    f = 1.0;
    if (particleSys)
        particleSys->Stop();
}
void ManaIcon::Drop()
{
    mode = DROPPING;
    if (f < 0)
        f = 0;
    if (particleSys)
        particleSys->Stop();
}

GuiMana::GuiMana(float x, float y, Player *p) :
    GuiLayer(p->getObserver()), x(x), y(y), owner(p)
{
}

GuiMana::~GuiMana()
{
    for (vector<ManaIcon*>::iterator it = manas.begin(); it != manas.end(); ++it)
    {
        delete (*it);
    }
}

void GuiMana::RenderStatic()
{
    vector<int> values;
    values.resize(Constants::NB_Colors);
    int totalColors = 0;
    WFont * mFont = WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);
    JRenderer * r = JRenderer::GetInstance();
    for (int i = 0; i < Constants::NB_Colors; ++i)
        values[i] = 0;
    for (vector<ManaIcon*>::iterator it = manas.begin(); it != manas.end(); ++it)
        if (ManaIcon::ALIVE == (*it)->mode)
        {
            values[(*it)->color]++;
            if (values[(*it)->color] == 1)
                totalColors++;
        }

    if (!totalColors)
        return;

    float x0 = x - 20 * totalColors;
    x0 = max(40.f, x0);
    float xEnd = x0 + 20 * totalColors;
    r->FillRoundRect(x0, y - 5, static_cast<float> (20 * totalColors + 5), 20, 2, ARGB(128,0,0,0));

    int offset = 0;
    for (int i = 0; i < Constants::NB_Colors; ++i)
    {
        if (values[i])
        {
            offset -= 20;
            r->RenderQuad(manaIcons[i].get(), xEnd + 15 + offset, y + 5, 0, 0.7f, 0.7f);
        }
    }
    r->FillRoundRect(x0, y, static_cast<float> (20 * totalColors + 5), 8, 2, ARGB(100,0,0,0));
    offset = 0;
    for (int i = 0; i < Constants::NB_Colors; ++i)
    {
        if (values[i])
        {
            offset -= 20;
            char buf[4];
            sprintf(buf, "%i", values[i]);
            mFont->SetColor(ARGB(255,255,255,255));
            mFont->DrawString(buf, xEnd + offset + 9, y);
        }
    }
}

void GuiMana::Render()
{
    if (manas.size() > 20)
    {
        int count = 0;
        for (vector<ManaIcon*>::iterator it = manas.begin(); it != manas.end(); ++it)
        {
            if (count > 20)
                break;
            count++;
            (*it)->Render();
        }
    }
    else
    for (vector<ManaIcon*>::iterator it = manas.begin(); it != manas.end(); ++it)
        (*it)->Render();

		if (OptionManaDisplay::DYNAMIC != options[Options::MANADISPLAY].number && OptionManaDisplay::NOSTARSDYNAMIC != options[Options::MANADISPLAY].number )
        RenderStatic();
}

bool remove_dead(ManaIcon* m)
{
    return ManaIcon::DEAD != m->mode;
}

void GuiMana::Update(float dt)
{
    if(observer->getResourceManager())
    {
        float shift = 0;
        for (vector<ManaIcon*>::iterator it = manas.begin(); it != manas.end(); ++it)
        {
            (*it)->Update(dt, shift);
            shift += 15;
        }
    }
    vector<ManaIcon*>::iterator it = partition(manas.begin(), manas.end(), &remove_dead);
    if (it != manas.end())
    {
        for (vector<ManaIcon*>::iterator q = it; q != manas.end(); ++q)
            SAFE_DELETE(*q);
        manas.erase(it, manas.end());
    }
}

int GuiMana::receiveEventPlus(WEvent* e)
{
    if (WEventEngageMana *event = dynamic_cast<WEventEngageMana*>(e))
    {
        if (event->destination != owner->getManaPool())
            return 0;
        if (event->card && event->card->view)
            manas.push_back(NEW ManaIcon(event->color, event->card->view->actX, event->card->view->actY, x, y));
        else
            manas.push_back(NEW ManaIcon(event->color, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, x, y));
        return 1;
    }
    else
        return 0;
}

int GuiMana::receiveEventMinus(WEvent* e)
{
    if (WEventConsumeMana *event = dynamic_cast<WEventConsumeMana*>(e))
    {
        if (event->source != owner->getManaPool())
            return 0;
        for (vector<ManaIcon*>::iterator it = manas.begin(); it != manas.end(); ++it)
            if ((event->color == (*it)->color) && (ManaIcon::ALIVE == (*it)->mode))
            {
                (*it)->Wither();
                return 1;
            }
        return 1;
    }
    else if (WEventEmptyManaPool *event2 = dynamic_cast<WEventEmptyManaPool*>(e))
    {
        if (event2->source != owner->getManaPool())
            return 0;
        for (vector<ManaIcon*>::iterator it = manas.begin(); it != manas.end(); ++it)
            (*it)->Drop();
        return 1;
    }
    return 0;
}
