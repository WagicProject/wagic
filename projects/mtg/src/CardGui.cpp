/*
* CardGui.cpp
* This class is in charge of rendering Cards on the screen
*/

#include "PrecompiledHeader.h"

#include "JGE.h"
#include "CardGui.h"
#include "ManaCostHybrid.h"
#include "ExtraCost.h"
#include "Subtypes.h"
#include "Translate.h"
#include "MTGDefinitions.h"
#include "GameObserver.h"
#include <Vector2D.h>
#include "Counters.h"
#include "ModRules.h"
#include "CardDescriptor.h"

const float CardGui::Width = 28.0;
const float CardGui::Height = 40.0;
const float CardGui::BigWidth = 200.0;
const float CardGui::BigHeight = 285.0;

const float kWidthScaleFactor = 0.8f;

map<string, string> CardGui::counterGraphics;

namespace
{
    inline float SineHelperFunction(const float& value)
    {
        return sinf(2 * M_PI * (value) / 256.0f);
    }

    inline float CosineHelperFunction(const float& value)
    {
        return cosf(2 * M_PI * (value - 35) / 256.0f);
    }

}

CardGui::CardGui(MTGCardInstance* card, float x, float y)
	: PlayGuiObject(Height, x, y, 0, false), card(card)
{
}
CardGui::CardGui(MTGCardInstance* card, const Pos& ref)
	: PlayGuiObject(Height, ref, 0, false), card(card)
{
}

float CardView::GetCenterX()
{
    bool largeCard = mHeight == BigHeight;

    float centerX = x + (largeCard ? BigWidth : Width) * 0.5f * zoom;
    return centerX;
}

float CardView::GetCenterY()
{
    bool largeCard = mHeight == BigHeight;

    float centerY = y + (largeCard ? BigHeight : Height) * 0.5f * zoom;
    return centerY;
}


CardView::CardView(const SelectorZone owner, MTGCardInstance* card, float x, float y)
	: CardGui(card, x, y), owner(owner)
{
    const Pos* ref = card->view;
    while (card)
    {
        if (ref == card->view)
            card->view = this;
        card = card->next;
    }
}

CardView::CardView(const SelectorZone owner, MTGCardInstance* card, const Pos& ref)
	: CardGui(card, ref), owner(owner)
{
    const Pos* r = card->view;
    while (card)
    {
        if (r == card->view)
            card->view = this;
        card = card->next;
    }
}

CardView::~CardView()
{
    if (card)
    {
        const Pos* r = this;
        while (card)
        {
            if (r == card->view)
                card->view = NULL;
            card = card->next;
        }
    }
}

void CardGui::Update(float dt)
{
    PlayGuiObject::Update(dt);
}

void CardGui::DrawCard(const Pos& inPosition, int inMode)
{
    DrawCard(card, inPosition, inMode);
}

void CardGui::DrawCard(MTGCard* inCard, const Pos& inPosition, int inMode)
{
    switch (inMode)
    {
    case DrawMode::kNormal:
        RenderBig(inCard, inPosition);
        break;
    case DrawMode::kText:
        AlternateRender(inCard, inPosition);
        break;
    default:
        break;
    }
}

void CardGui::Render()
{
    GameObserver * game = card->getObserver();
    WFont * mFont = game?game->getResourceManager()->GetWFont(Fonts::MAIN_FONT):WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);
    JRenderer * renderer = JRenderer::GetInstance();
    TargetChooser * tc = NULL;

    if (game)
        tc = game->getCurrentTargetChooser();

    bool alternate = true;
    JQuadPtr quad = game? game->getResourceManager()->RetrieveCard(card, CACHE_THUMB):WResourceManager::Instance()->RetrieveCard(card, CACHE_THUMB);
    if(card && !card->isToken && card->name != card->model->data->name)
    {
        MTGCard * fcard = MTGCollection()->getCardByName(card->name);
        quad = game->getResourceManager()->RetrieveCard(fcard, CACHE_THUMB);
    }
    if (quad.get())
        alternate = false;
    else
        quad = AlternateThumbQuad(card);

    float cardScale = quad ? 40 / quad->mHeight : 1;
    float scale = actZ * cardScale;

    JQuadPtr shadow;
    if (actZ > 1)
    {
        shadow = game? game->getResourceManager()->GetQuad("shadow"):WResourceManager::Instance()->GetQuad("shadow");
        if (shadow) 
        {
            shadow->SetColor(ARGB(static_cast<unsigned char>(actA)/2,255,255,255));
            renderer->RenderQuad(shadow.get(), actX + (actZ - 1) * 15, actY + (actZ - 1) * 15, actT, 28 * actZ / 16, 40 * actZ / 16);
        }
    }

    JQuadPtr extracostshadow;
    if (card->isExtraCostTarget)
    {
        extracostshadow = card->getObserver()->getResourceManager()->GetQuad("extracostshadow");
        if (extracostshadow) 
        {
            extracostshadow->SetColor(ARGB(static_cast<unsigned char>(actA)/2,100,0,0));
            renderer->RenderQuad(extracostshadow.get(), actX + (actZ - 1) * 15, actY + (actZ - 1) * 15, actT, 28 * actZ / 16, 40 * actZ / 16);
        }
    }

    if(game && game->connectRule)
    {
        // Am I a parent of a selected card, or am I a parent and myself being selected?
        bool isActiveConnectedParent = mHasFocus && card->childrenCards.size();
        if (!isActiveConnectedParent)
        {
            for (size_t i = 0; i < card->childrenCards.size(); ++i)
            {
                MTGCardInstance * child = card->childrenCards[i];
                if (CardView* cv = dynamic_cast<CardView*>(child->view))
                {
                    if (cv->mHasFocus)
                    {
                        isActiveConnectedParent = true;
                        break;
                    }
                }
            }
        }
        if (isActiveConnectedParent)
        {
            JQuadPtr white = card->getObserver()->getResourceManager()->GetQuad("white");
            if(white)
            {
                white->SetColor(ARGB(255,230,50,50));
                renderer->RenderQuad(white.get(), actX, actY, actT, 30 * actZ / 16, 42 * actZ / 16);
            }
        }

        // Am I a child of a selected card, or am I a child and myself being selected?
        bool isActiveConnectedChild = mHasFocus && card->parentCards.size();
        if (!isActiveConnectedChild)
        {
            for (size_t i = 0; i < card->parentCards.size(); ++i)
            {
                MTGCardInstance * parent = card->parentCards[i];
                if (CardView* cv = dynamic_cast<CardView*>(parent->view))
                {
                    if (cv->mHasFocus)
                    {
                        isActiveConnectedChild = true;
                        break;
                    }
                }
            }
        }
        if (isActiveConnectedChild)
        {
            JQuadPtr white = card->getObserver()->getResourceManager()->GetQuad("white");
            if(white)
            {
                white->SetColor(ARGB(255,0,0,255));
                renderer->RenderQuad(white.get(), actX, actY, actT, 30 * actZ / 16, 42 * actZ / 16);
            }
        }
    }
    if (quad)
    {
        quad->SetColor(ARGB(static_cast<unsigned char>(actA),255,255,255));
        renderer->RenderQuad(quad.get(), actX, actY, actT, scale, scale);
    }

    if (alternate)
    {
        mFont->SetColor(ARGB(static_cast<unsigned char>(actA), 0, 0, 0));
        mFont->SetScale(DEFAULT_MAIN_FONT_SCALE * 0.5f * actZ);
        mFont->DrawString(_(card->getName()), actX - actZ * Width / 2 + 1, actY - actZ * Height / 2 + 1);
        mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);

        JQuadPtr icon;
        if (card->hasSubtype("plains"))
            icon = game?game->getResourceManager()->GetQuad("c_white"):WResourceManager::Instance()->GetQuad("c_white");
        else if (card->hasSubtype("swamp"))
            icon = game?game->getResourceManager()->GetQuad("c_black"):WResourceManager::Instance()->GetQuad("c_black");
        else if (card->hasSubtype("forest"))
            icon = game?game->getResourceManager()->GetQuad("c_green"):WResourceManager::Instance()->GetQuad("c_green");
        else if (card->hasSubtype("mountain"))
            icon = game?game->getResourceManager()->GetQuad("c_red"):WResourceManager::Instance()->GetQuad("c_red");
        else if (card->hasSubtype("island"))
            icon = game?game->getResourceManager()->GetQuad("c_blue"):WResourceManager::Instance()->GetQuad("c_blue");
        if (icon.get())
        {
            icon->SetColor(ARGB(static_cast<unsigned char>(actA),255,255,255));
            renderer->RenderQuad(icon.get(), actX, actY, actT);
            icon->SetColor(ARGB(255,255,255,255)); //Putting color back as this quad is shared
        }

    }
    JQuadPtr mor;
    if(card->isMorphed && !alternate)
    {
        mor = card->getObserver()->getResourceManager()->RetrieveTempQuad("morph.jpg");
        if (mor &&  mor->mTex) {
            mor->SetHotSpot(static_cast<float> (mor->mTex->mWidth / 2), static_cast<float> (mor->mTex->mHeight / 2));
            mor->SetColor(ARGB(255,255,255,255));
            renderer->RenderQuad(mor.get(), actX, actY, actT,scale, scale);
        }
    }

    //draws the numbers power/toughness
    if (card->isCreature())
    {
        mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
        char buffer[200];
        sprintf(buffer, "%i/%i", card->power, card->life);
        renderer->FillRect(actX - (12 * actZ), actY + 6 * actZ, 25 * actZ, 12 * actZ,
            ARGB(((static_cast<unsigned char>(actA))/2),0,0,0));
        mFont->SetColor(ARGB(static_cast<unsigned char>(actA),255,255,255));
        mFont->SetScale(actZ);
        mFont->DrawString(buffer, actX - 10 * actZ, actY + 8 * actZ);
        mFont->SetScale(1);
    }

    if (card->counters->mCount > 0)
    {
        unsigned c = -1;
        for (int i = 0; i < card->counters->mCount; i++)
        {
            if (card->counters->counters[i]->name != "")
                c = i;
            break;
        }
        if (c + 1)
        {
            mFont->SetScale(DEFAULT_MAIN_FONT_SCALE);
            char buffer[200];
            sprintf(buffer, "%i", card->counters->counters[0]->nb);
            mFont->SetColor(ARGB(static_cast<unsigned char>(actA),255,255,255));
            mFont->SetScale(actZ);
            mFont->DrawString(buffer, actX - 10 * actZ, actY - (12 * actZ));
            mFont->SetScale(1);
        }
    }

    if (tc && !tc->canTarget(card))
    {
        if (!shadow)
            shadow = card->getObserver()->getResourceManager()->GetQuad("shadow");
        if (shadow)
        {
            shadow->SetColor(ARGB(200,255,255,255));
            renderer->RenderQuad(shadow.get(), actX, actY, actT, (28 * actZ + 1) / 16, 40 * actZ / 16);
        }
    }

    // Render a mask over the card, if set
    if (mask && quad)
        JRenderer::GetInstance()->FillRect(actX - (scale * quad->mWidth / 2),actY - (scale * quad->mHeight / 2), scale * quad->mWidth, scale* quad->mHeight, mask);

    if ((tc && tc->alreadyHasTarget(card)) || (game && card == game->mLayers->actionLayer()->currentActionCard))//paint targets red.
    {
        if (card->isTapped())
        {
            renderer->FillRect(actX - (scale * quad->mWidth / 2)-7,actY - (scale * quad->mHeight / 2)+7,scale* quad->mHeight,scale * quad->mWidth, ARGB(128,255,0,0));
        }
        else
        {
            renderer->FillRect(actX - (scale * quad->mWidth / 2),actY - (scale * quad->mHeight / 2), scale * quad->mWidth, scale* quad->mHeight, ARGB(128,255,0,0));
        }
    }
    if(tc && tc->source && tc->source->view && tc->source->view->actZ >= 1.3 && card == tc->source)//paint the source green while infocus.
    {
        if (tc->source->isTapped())
        {
            renderer->FillRect(actX - (scale * quad->mWidth / 2)-7,actY - (scale * quad->mHeight / 2)+7,scale* quad->mHeight,scale * quad->mWidth, ARGB(128,0,255,0));
        }
        else
        {
            renderer->FillRect(tc->source->view->actX - (scale * quad->mWidth / 2),tc->source->view->actY - (scale * quad->mHeight / 2), scale*quad->mWidth, scale*quad->mHeight, ARGB(128,0,255,0));
        }
    }

    PlayGuiObject::Render();
}

JQuadPtr CardGui::AlternateThumbQuad(MTGCard * card)
{
    JQuadPtr q;
    vector<ModRulesBackGroundCardGuiItem *>items = gModRules.cardgui.background;
    ModRulesBackGroundCardGuiItem * item;
    int numItems = (int)items.size();
    if (card->data->countColors() > 1)
    {
         item = items[numItems-1];
    }
    else
    {
        item = items[card->data->getColor()];
    }
    
    
    q = WResourceManager::Instance()->RetrieveTempQuad(item->mDisplayThumb);
    items.clear();  
    if (q && q->mTex)
        q->SetHotSpot(static_cast<float> (q->mTex->mWidth / 2), static_cast<float> (q->mTex->mHeight / 2));
    return q;
}

void CardGui::AlternateRender(MTGCard * card, const Pos& pos)
{
    // Draw the "unknown" card model
    JRenderer * renderer = JRenderer::GetInstance();
    JQuadPtr q;

    float x = pos.actX;
   
    vector<ModRulesBackGroundCardGuiItem *>items = gModRules.cardgui.background;
    ModRulesBackGroundCardGuiItem * item;
    int numItems = (int)items.size();
    if (card->data->countColors() > 1)
    {
         item = items[numItems-1];
    }
    else
    {
        item = items[card->data->getColor()];
    }
    
    q = WResourceManager::Instance()->RetrieveTempQuad(item->mDisplayImg,TEXTURE_SUB_5551);

    items.clear();
    if (q.get() && q->mTex)
    {
        q->SetHotSpot(static_cast<float> (q->mTex->mWidth / 2), static_cast<float> (q->mTex->mHeight / 2));

        float scale = pos.actZ * 250 / q->mHeight;
        q->SetColor(ARGB((int)pos.actA,255,255,255));
        renderer->RenderQuad(q.get(), x, pos.actY, pos.actT, scale, scale);
    }

    vector<ModRulesRenderCardGuiItem *>Carditems = gModRules.cardgui.renderbig;
    
    WFont * font = WResourceManager::Instance()->GetWFont(Fonts::MAGIC_FONT);
    float backup_scale = font->GetScale();
    font->SetColor(ARGB((int)pos.actA, 0, 0, 0));
    string sFormattedData = "";
    
    for( size_t i =0 ; i < Carditems.size(); i ++)
    {
        ModRulesRenderCardGuiItem * Carditem = Carditems[i];
        if (Carditem->mFilter.length() == 0 || FilterCard(card,Carditem->mFilter.c_str()))
        {

           if (Carditem->mFont) 
            {
                font->SetColor(Carditem->mFontColor);
                font->SetScale(((float)Carditem->mFontSize / 100) * pos.actZ);
                
            }
            else
            {
                font->SetColor(ARGB((int)pos.actA, 0, 0, 0));
                font->SetScale(kWidthScaleFactor * pos.actZ);

            }
            
            if (Carditem->mName == "description")
            {

                std::vector<string> txt = card->data->getFormattedText();

                unsigned i = 0;
                unsigned h = neofont ? 14 : 11;
                for (std::vector<string>::const_iterator it = txt.begin(); it != txt.end(); ++it, ++i)
                    font->DrawString(_(it->c_str()), x + (Carditem->mPosX - BigWidth / 2) * pos.actZ, pos.actY + (-BigHeight / 2 + Carditem->mPosY + h * i) * pos.actZ);
            }
            else if (Carditem->mName == "mana")
            {
                // Mana
                // Need Create a realy generic struct for mana render
                ManaCost* manacost = card->data->getManaCost();
                ManaCostHybrid* h;
                ExtraCost* e;
                unsigned int j = 0;
                unsigned int z = 0;
                unsigned char t = (JGE::GetInstance()->GetTime() / 3) & 0xFF;
                unsigned char v = t + 127;
                float yOffset = (float)Carditem->mPosY;
                
                JQuadPtr quad = WResourceManager::Instance()->RetrieveQuad("menuicons.png", 0, 0, 0, 0, "", RETRIEVE_NORMAL, TEXTURE_SUB_5551, 1);
                if (quad.get())
                    if (quad->mHeight >= 78)
                    while ((e = manacost->getExtraCost(z)))
                    {
                            if(e->mCostRenderString == "Phyrexian Mana")
                            {
                                float _color = (float)card->data->getColor() -1;
                                JQuadPtr ExtraManas = WResourceManager::Instance()->RetrieveQuad("menuicons.png", 2 + _color * 36, 76, 32, 32, "c_extra", RETRIEVE_MANAGE);
                                ExtraManas->SetHotSpot(16, 16);
                                renderer->RenderQuad(ExtraManas.get(), x + (-12 * j + Carditem->mPosX) * pos.actZ, pos.actY + (yOffset) * pos.actZ, 0, 0.4f
                                * pos.actZ, 0.4f * pos.actZ);
                            }
                   
                        ++j;
                        ++z;
                    }
               
                z=0;
                while ((h = manacost->getHybridCost(z)))
                {
                    float scale = pos.actZ * 0.05f * cosf(2 * M_PI * ((float) t) / 256.0f);

                    if (scale < 0)
                    {
                        renderer->RenderQuad(manaIcons[h->color1].get(), x + (-12 * j + Carditem->mPosX + 3 * SineHelperFunction((float) t)) * pos.actZ,
                            pos.actY + (yOffset + 3 * CosineHelperFunction((float) t)) * pos.actZ, 0, 0.4f + scale, 0.4f
                            + scale);
                        renderer->RenderQuad(manaIcons[h->color2].get(), x + (-12 * j + Carditem->mPosX + 3 * SineHelperFunction((float) v)) * pos.actZ,
                            pos.actY + (yOffset + 3 * CosineHelperFunction((float) v)) * pos.actZ, 0, 0.4f - scale, 0.4f
                            - scale);
                    }
                    else
                    {
                        renderer->RenderQuad(manaIcons[h->color2].get(), x + (-12 * j + Carditem->mPosX + 3 * SineHelperFunction((float) v)) * pos.actZ,
                            pos.actY + (yOffset + 3 * CosineHelperFunction((float) v)) * pos.actZ, 0, 0.4f - scale, 0.4f
                            - scale);
                        renderer->RenderQuad(manaIcons[h->color1].get(), x + (-12 * j + Carditem->mPosX + 3 * SineHelperFunction((float) t)) * pos.actZ,
                            pos.actY + (yOffset + 3 * CosineHelperFunction((float) t)) * pos.actZ, 0, 0.4f + scale, 0.4f
                            + scale);
                    }
                    ++j;
                    ++z;
                }
                for (int i = Constants::NB_Colors - 2; i >= 1; --i)
                {
                     int cost;
                    for (cost = manacost->getCost(i); cost > 0; --cost)
                    {
                        renderer->RenderQuad(manaIcons[i].get(), x + (-12 * j + Carditem->mPosX) * pos.actZ, pos.actY + (yOffset) * pos.actZ, 0, 0.4f
                            * pos.actZ, 0.4f * pos.actZ);
                        ++j;
                    }
                    
                }
                // Colorless mana
                if (int cost = manacost->getCost(0))
                {
                    char buffer[10];
                    sprintf(buffer, "%d", cost);
                    renderer->RenderQuad(manaIcons[0].get(), x + (-12 * j + Carditem->mPosX) * pos.actZ, pos.actY + (yOffset) * pos.actZ, 0, 0.4f * pos.actZ,
                        0.4f * pos.actZ);
                    float w = font->GetStringWidth(buffer);
                    font->DrawString(buffer, x + (-12 * j + (Carditem->mPosX +1) - w / 2) * pos.actZ, pos.actY + (yOffset - 5) * pos.actZ);
                    ++j;
                }
                //Has X?
                if (manacost->hasX())
                {
                    char buffer[10];
                    sprintf(buffer, "X");
                    renderer->RenderQuad(manaIcons[0].get(), x + (-12 * j + Carditem->mPosX) * pos.actZ, pos.actY + (yOffset) * pos.actZ, 0, 0.4f * pos.actZ,
                        0.4f * pos.actZ);
                    float w = font->GetStringWidth(buffer);
                    font->DrawString(buffer, x + (-12 * j + (Carditem->mPosX + 1) - w / 2) * pos.actZ, pos.actY + (yOffset - 5) * pos.actZ);
                }
    
            }
            else if (Carditem->mName == "icon")
            {
                float yOffseticon = (float)Carditem->mPosY;
                JQuadPtr ExtraIcons = WResourceManager::Instance()->RetrieveQuad(Carditem->mFileName.c_str(), 2 + (float)(Carditem->mIconPosX - 1) * 36, (float)(Carditem->mIconPosY -1) * 38 , 32, 32, "", RETRIEVE_MANAGE);
                ExtraIcons->SetHotSpot(16,16);
                renderer->RenderQuad(ExtraIcons.get(), x + (Carditem->mPosX) * pos.actZ, pos.actY + (yOffseticon) * pos.actZ, 0, (float)Carditem->mSizeIcon * 0.4f
                * pos.actZ, (float)Carditem->mSizeIcon* 0.4f * pos.actZ);

            }
            else 
            {
                string formattedfield = Carditem->mFormattedData;
                size_t found = Carditem->mName.find("title"); // Write the title
                if (found != string::npos)
                {
                    stringstream st;
                    st << _(card->data->name);
                    formattedfield = FormattedData(formattedfield, "title", st.str());
                
                }

                found = Carditem->mName.find("power"); // Write the strength
                if (found != string::npos)
                {
                    stringstream st;
                    st << card->data->power;
                    formattedfield = FormattedData(formattedfield, "power", st.str());
                }
                found = Carditem->mName.find("life"); // Write the toughness
                if (found != string::npos)
                {
                    stringstream st;
                    st << card->data->toughness;
                    formattedfield = FormattedData(formattedfield, "life", st.str());

                }

                found = Carditem->mName.find("types"); //types
                if (found != string::npos)
                {
                    string s = "";
                    for (int i = card->data->types.size() - 1; i > 0; --i)
                    {
                        if (card->data->basicAbilities[(int)Constants::CHANGELING])
                        {// this avoids drawing the list of subtypes on changeling cards.
                            s += _("Shapeshifter - ");
                            break;
                        }
                        else
                        {
                            s += _(MTGAllCards::findType(card->data->types[i]));
                            s += _(" - ");
                        }
                    }
                    if (card->data->types.size())
                        s += _(MTGAllCards::findType(card->data->types[0]));
                    else
                    {
                        DebugTrace("Typeless card: " << setlist[card->setId].c_str() << card->data->getName() << card->getId());
                    }

                    formattedfield = FormattedData(formattedfield, "types", s);
                }

                found = Carditem->mName.find("rarity");
                if (found != string::npos)
                {
                    
                    string sRarity;
                    switch(card->getRarity())
                    {
                    case Constants::RARITY_M:
                        sRarity =_("Mythic");
                        break;
                    case Constants::RARITY_R:
                        sRarity =_("Rare");
                        break;
                    case Constants::RARITY_U:
                        sRarity =_("Uncommon");
                        break;
                    case Constants::RARITY_C:
                        sRarity =_("Common");
                        break;
                    case Constants::RARITY_L:
                        sRarity =_("Land");
                        break;
                    case Constants::RARITY_T:
                        sRarity =_("Token");
                        break;
                    default:
                    case Constants::RARITY_S:
                        sRarity =_("Special");
                        break;
                    }
                    formattedfield = FormattedData(formattedfield, "rarity", sRarity);
                }

                 found = Carditem->mName.find("expansion");
                if (found != string::npos)
                {
                    formattedfield = FormattedData(formattedfield, "expansion", setlist[card->setId].c_str());
                }

                if (!Carditem->mFont) 
                {          
                    float w = font->GetStringWidth(formattedfield.c_str()) * kWidthScaleFactor * pos.actZ;
                    if (w > BigWidth - 30)
                        font->SetScale((BigWidth - 30) / w);
                }
                font->DrawString(formattedfield.c_str(), x + (Carditem->mPosX  - BigWidth / 2) * pos.actZ, pos.actY + (Carditem->mPosY - BigHeight / 2) * pos.actZ);
            
            }
         
        }
    }


    
    font->SetScale(backup_scale);

    RenderCountersBig(card, pos, DrawMode::kText);
}

void CardGui::TinyCropRender(MTGCard * card, const Pos& pos, JQuad * quad)
{
    if (!quad)
        return;

    JRenderer * renderer = JRenderer::GetInstance();
    JQuadPtr q;

    float x = pos.actX;
    float displayScale = 250 / BigHeight;
   
    vector<ModRulesBackGroundCardGuiItem *>items = gModRules.cardgui.background;
    ModRulesBackGroundCardGuiItem * item;
    int numItems = (int)items.size();
    if (card->data->countColors() > 1)
    {
         item = items[numItems-1];
    }
    else
    {
        item = items[card->data->getColor()];
    }
    
    q = WResourceManager::Instance()->RetrieveTempQuad(item->mDisplayImg,TEXTURE_SUB_5551);
    items.clear();
    if (q.get() && q->mTex)
    {
        q->SetHotSpot(static_cast<float> (q->mTex->mWidth / 2), static_cast<float> (q->mTex->mHeight / 2));

        float scale = pos.actZ * displayScale * BigHeight / q->mHeight;
        q->SetColor(ARGB((int)pos.actA,255,255,255));
        renderer->RenderQuad(q.get(), x, pos.actY, pos.actT, scale, scale);
    }
    
    std::vector<string> txt = card->data->getFormattedText();
    size_t nbTextLines = txt.size();

    //Render the image on top of that
    quad->SetColor(ARGB((int)pos.actA,255,255,255));
    float imgScale = pos.actZ * (displayScale * (BigWidth - 15)) / quad->mWidth;
    float imgY = pos.actY - (20 * imgScale);
    if (nbTextLines > 6)
    {
        imgY -= 10 * imgScale;
        imgScale *= 0.75;
    }
    renderer->RenderQuad(quad, x, imgY, pos.actT, imgScale, imgScale);



    vector<ModRulesRenderCardGuiItem *>Carditems = gModRules.cardgui.rendertinycrop;
    
    WFont * font = WResourceManager::Instance()->GetWFont(Fonts::MAGIC_FONT);
    float backup_scale = font->GetScale();
    string sFormattedData = "";
    for( size_t i =0 ; i < Carditems.size(); i ++)
    {
        ModRulesRenderCardGuiItem * Carditem = Carditems[i];
        if (Carditem->mFilter.length() == 0 || FilterCard(card,Carditem->mFilter.c_str()))
        {

           if (Carditem->mFont) 
            {
                font->SetColor(Carditem->mFontColor);
                font->SetScale(((float)Carditem->mFontSize / 100) * pos.actZ);
                    
            }
            else
            {
                font->SetColor(ARGB((int)pos.actA, 0, 0, 0));
                font->SetScale(kWidthScaleFactor * pos.actZ);

            }
            
            if (Carditem->mName == "description")
            {

                std::vector<string> txt = card->data->getFormattedText();
                float imgBottom = imgY + (imgScale * quad->mHeight / 2);

                unsigned i = 0;
                unsigned h = neofont ? 14 : 11;
                for (std::vector<string>::const_iterator it = txt.begin(); it != txt.end(); ++it, ++i)
                    font->DrawString(it->c_str(), x + (Carditem->mPosX - BigWidth / 2) * pos.actZ, imgBottom + (Carditem->mPosY + h * i) * pos.actZ);
            }
            else if (Carditem->mName == "mana")
            {
                // Mana
                // Need Create a realy generic struct for mana render
                ManaCost* manacost = card->data->getManaCost();
                ManaCostHybrid* h;
                ExtraCost* e;
                unsigned int j = 0;
                unsigned int z = 0;
                unsigned char t = (JGE::GetInstance()->GetTime() / 3) & 0xFF;
                unsigned char v = t + 127;
                float yOffset = (float)Carditem->mPosY;
                
                JQuadPtr quad = WResourceManager::Instance()->RetrieveQuad("menuicons.png", 0, 0, 0, 0, "", RETRIEVE_NORMAL, TEXTURE_SUB_5551, 1);
                if (quad.get())
                    if (quad->mHeight >= 78)
                    while ((e = manacost->getExtraCost(z)))
                    {
                            if(e->mCostRenderString == "Phyrexian Mana")
                            {
                                float _color = (float)card->data->getColor() -1;
                                JQuadPtr ExtraManas = WResourceManager::Instance()->RetrieveQuad("menuicons.png", 2 + _color * 36, 76, 32, 32, "c_extra", RETRIEVE_MANAGE);
                                ExtraManas->SetHotSpot(16, 16);
                                renderer->RenderQuad(ExtraManas.get(), x + (-12 * j + Carditem->mPosX) * pos.actZ, pos.actY + (yOffset) * pos.actZ, 0, 0.4f
                                * pos.actZ, 0.4f * pos.actZ);
                            }
                   
                        ++j;
                        ++z;
                    }
               
                z=0;
                while ((h = manacost->getHybridCost(z)))
                {
                    float scale = pos.actZ * 0.05f * cosf(2 * M_PI * ((float) t) / 256.0f);

                    if (scale < 0)
                    {
                        renderer->RenderQuad(manaIcons[h->color1].get(), x + (-12 * j + Carditem->mPosX + 3 * SineHelperFunction((float) t)) * pos.actZ,
                            pos.actY + (yOffset + 3 * CosineHelperFunction((float) t)) * pos.actZ, 0, 0.4f + scale, 0.4f
                            + scale);
                        renderer->RenderQuad(manaIcons[h->color2].get(), x + (-12 * j + Carditem->mPosX + 3 * SineHelperFunction((float) v)) * pos.actZ,
                            pos.actY + (yOffset + 3 * CosineHelperFunction((float) v)) * pos.actZ, 0, 0.4f - scale, 0.4f
                            - scale);
                    }
                    else
                    {
                        renderer->RenderQuad(manaIcons[h->color2].get(), x + (-12 * j + Carditem->mPosX + 3 * SineHelperFunction((float) v)) * pos.actZ,
                            pos.actY + (yOffset + 3 * CosineHelperFunction((float) v)) * pos.actZ, 0, 0.4f - scale, 0.4f
                            - scale);
                        renderer->RenderQuad(manaIcons[h->color1].get(), x + (-12 * j + Carditem->mPosX + 3 * SineHelperFunction((float) t)) * pos.actZ,
                            pos.actY + (yOffset + 3 * CosineHelperFunction((float) t)) * pos.actZ, 0, 0.4f + scale, 0.4f
                            + scale);
                    }
                    ++j;
                    ++z;
                }
                for (int i = Constants::NB_Colors - 2; i >= 1; --i)
                {
                     int cost;
                    for (cost = manacost->getCost(i); cost > 0; --cost)
                    {
                        renderer->RenderQuad(manaIcons[i].get(), x + (-12 * j + Carditem->mPosX) * pos.actZ, pos.actY + (yOffset) * pos.actZ, 0, 0.4f
                            * pos.actZ, 0.4f * pos.actZ);
                        ++j;
                    }
                    
                }
                // Colorless mana
                if (int cost = manacost->getCost(0))
                {
                    char buffer[10];
                    sprintf(buffer, "%d", cost);
                    renderer->RenderQuad(manaIcons[0].get(), x + (-12 * j + Carditem->mPosX) * pos.actZ, pos.actY + (yOffset) * pos.actZ, 0, 0.4f * pos.actZ,
                        0.4f * pos.actZ);
                    float w = font->GetStringWidth(buffer);
                    font->DrawString(buffer, x + (-12 * j + (Carditem->mPosX +1) - w / 2) * pos.actZ, pos.actY + (yOffset - 5) * pos.actZ);
                    ++j;
                }
                //Has X?
                if (manacost->hasX())
                {
                    char buffer[10];
                    sprintf(buffer, "X");
                    renderer->RenderQuad(manaIcons[0].get(), x + (-12 * j + Carditem->mPosX) * pos.actZ, pos.actY + (yOffset) * pos.actZ, 0, 0.4f * pos.actZ,
                        0.4f * pos.actZ);
                    float w = font->GetStringWidth(buffer);
                    font->DrawString(buffer, x + (-12 * j + (Carditem->mPosX + 1) - w / 2) * pos.actZ, pos.actY + (yOffset - 5) * pos.actZ);
                }
    
            }
            else if (Carditem->mName == "icon")
            {
                float yOffseticon = (float)Carditem->mPosY;
                JQuadPtr ExtraIcons = WResourceManager::Instance()->RetrieveQuad(Carditem->mFileName.c_str(), 2 + (float)(Carditem->mIconPosX - 1) * 36, (float)(Carditem->mIconPosY -1) * 38 , 32, 32, "", RETRIEVE_MANAGE);
                ExtraIcons->SetHotSpot(16,16);
                renderer->RenderQuad(ExtraIcons.get(), x + (Carditem->mPosX) * pos.actZ, pos.actY + (yOffseticon) * pos.actZ, 0, (float)Carditem->mSizeIcon * 0.4f
                * pos.actZ, (float)Carditem->mSizeIcon* 0.4f * pos.actZ);

            }
            else 
            {
                string formattedfield = Carditem->mFormattedData;
                size_t found = Carditem->mName.find("title"); // Write the title
                if (found != string::npos)
                {
                    stringstream st;
                    st << card->data->name;
                    formattedfield = FormattedData(formattedfield, "title", st.str());
                
                }

                found = Carditem->mName.find("power"); // Write the strength
                if (found != string::npos)
                {
                    stringstream st;
                    st << card->data->power;
                    formattedfield = FormattedData(formattedfield, "power", st.str());
                }
                found = Carditem->mName.find("life"); // Write the toughness
                if (found != string::npos)
                {
                    stringstream st;
                    st << card->data->toughness;
                    formattedfield = FormattedData(formattedfield, "life", st.str());

                }

                found = Carditem->mName.find("types"); //types
                if (found != string::npos)
                {
                    string s = "";
                    for (int i = card->data->types.size() - 1; i > 0; --i)
                    {
                        if (card->data->basicAbilities[(int)Constants::CHANGELING])
                        {// this avoids drawing the list of subtypes on changeling cards.
                            s += _("Shapeshifter - ");
                            break;
                        }
                        else
                        {
                            s += _(MTGAllCards::findType(card->data->types[i]));
                            s += _(" - ");
                        }
                    }
                    if (card->data->types.size())
                        s += _(MTGAllCards::findType(card->data->types[0]));
                    else
                    {
                        DebugTrace("Typeless card: " << setlist[card->setId].c_str() << card->data->getName() << card->getId());
                    }

                    formattedfield = FormattedData(formattedfield, "types", s);
                }

                found = Carditem->mName.find("rarity");
                if (found != string::npos)
                {
                    
                    string sRarity;
                    switch(card->getRarity())
                    {
                    case Constants::RARITY_M:
                        sRarity ="Mythic";
                        break;
                    case Constants::RARITY_R:
                        sRarity ="Rare";
                        break;
                    case Constants::RARITY_U:
                        sRarity ="Uncommon";
                        break;
                    case Constants::RARITY_C:
                        sRarity ="Common";
                        break;
                    case Constants::RARITY_L:
                        sRarity ="Land";
                        break;
                    case Constants::RARITY_T:
                        sRarity ="Token";
                        break;
                    default:
                    case Constants::RARITY_S:
                        sRarity ="Special";
                        break;
                    }
                    formattedfield = FormattedData(formattedfield, "rarity", sRarity);
                }

                 found = Carditem->mName.find("expansion");
                if (found != string::npos)
                {
                    formattedfield = FormattedData(formattedfield, "expansion", setlist[card->setId].c_str());
                }

                if (!Carditem->mFont) 
                {          
                    float w = font->GetStringWidth(formattedfield.c_str()) * kWidthScaleFactor * pos.actZ;
                    if (w > BigWidth - 30)
                        font->SetScale((BigWidth - 30) / w);
                }
                font->DrawString(formattedfield.c_str(), x + (Carditem->mPosX  - BigWidth / 2) * pos.actZ, pos.actY + (Carditem->mPosY - BigHeight / 2) * pos.actZ);
            
            }
         
        }
    }


    
    font->SetScale(backup_scale);

    RenderCountersBig(card, pos);
}

//Renders a big card on screen. Defaults to the "alternate" rendering if no image is found
void CardGui::RenderBig(MTGCard* card, const Pos& pos)
{
    JRenderer * renderer = JRenderer::GetInstance();
    //GameObserver * game = GameObserver::GetInstance();
    //if((MTGCard*)game->mLayers->actionLayer()->currentActionCard != NULL)
    //    card = (MTGCard*)game->mLayers->actionLayer()->currentActionCard;
    //i want this but ai targets cards so quickly that it can crash the game.
    float x = pos.actX;

    JQuadPtr quad = WResourceManager::Instance()->RetrieveCard(card);
    MTGCardInstance * kcard =  dynamic_cast<MTGCardInstance*>(card);
    if(kcard && !kcard->isToken && kcard->name != kcard->model->data->name)
    {
        MTGCard * fcard = MTGCollection()->getCardByName(kcard->name);
        quad = WResourceManager::Instance()->RetrieveCard(fcard);
    }
    if (quad.get())
    {
        if (quad->mHeight < quad->mWidth)
        {
            return TinyCropRender(card, pos, quad.get());
        }
        quad->SetColor(ARGB(255,255,255,255));
        float scale = pos.actZ * 250.f / quad->mHeight;
        renderer->RenderQuad(quad.get(), x, pos.actY, pos.actT, scale, scale);
        RenderCountersBig(card, pos);
        return;
    }

    //DebugTrace("Unable to fetch image: " << card->getImageName());

    // If we come here, we do not have the picture.
    AlternateRender(card, pos);
}

string CardGui::FormattedData(string data, string replace, string value)
{
    size_t found = data.find(replace.c_str());
    if (found != string::npos)
    {
        size_t len = replace.length();
        string teste = data.replace(found,len,value);
        return teste;
    }
    else
    {
        return value;
    }

}

bool CardGui::FilterCard(MTGCard * _card,string filter)
{
    CardDescriptor  cd;
    MTGCardInstance * card = (MTGCardInstance*) _card->data;
    cd.init();
    cd.mode = CD_OR;
    while (filter.size())
    {
        
        string typeName;
        //Advanced cards caracteristics ?
         size_t found = filter.find("[");
        if (found != string::npos)
        {
            int nbminuses = 0;
            int end = filter.find("]");
            string attributes = filter.substr(found + 1, end - found - 1);
            
            while (attributes.size())
            {
                size_t found2 = attributes.find(";");
                size_t foundAnd = attributes.find("&");
                string attribute;
                if (found2 != string::npos)
                {
                    cd.mode = CD_OR;
                    attribute = attributes.substr(0, found2);
                    attributes = attributes.substr(found2 + 1);
                }
                else if (foundAnd != string::npos)
                {
                    cd.mode = CD_AND;
                    attribute = attributes.substr(0, foundAnd);
                    attributes = attributes.substr(foundAnd + 1);
                }
                else
                {
                    attribute = attributes;
                    attributes = "";
                }
                int minus = 0;
                if (attribute[0] == '-')
                {
                    minus = 1;
                    nbminuses++;
                    attribute = attribute.substr(1);
                }
                int comparisonMode = COMPARISON_NONE;
                int comparisonCriterion = 0;
                if (attribute.size() > 1)
                {
                    size_t operatorPosition = attribute.find("=", 1);
                    if (operatorPosition != string::npos)
                    {
                        string numberCD = attribute.substr(operatorPosition + 1, attribute.size() - operatorPosition - 1);
                        
                        switch (attribute[operatorPosition - 1])
                        {
                        case '<':
                            if (minus)
                            {
                                comparisonMode = COMPARISON_GREATER;
                            }
                            else
                            {
                                comparisonMode = COMPARISON_AT_MOST;
                            }
                            operatorPosition--;
                            break;
                        case '>':
                            if (minus)
                            {
                                comparisonMode = COMPARISON_LESS;
                            }
                            else
                            {
                                comparisonMode = COMPARISON_AT_LEAST;
                            }
                            operatorPosition--;
                            break;
                        default:
                            if (minus)
                            {
                                comparisonMode = COMPARISON_UNEQUAL;
                            }
                            else
                            {
                                comparisonMode = COMPARISON_EQUAL;
                            }
                        }
                        attribute = attribute.substr(0, operatorPosition);
                    }
                }

                //Attacker
                if (attribute.find("attacking") != string::npos)
                {
                    if (minus)
                    {
                        cd.attacker = -1;
                    }
                    else
                    {
                        cd.attacker = 1;
                    }
                }
                //Blocker
                else if (attribute.find("blocking") != string::npos)
                {
                    if (minus)
                    {
                        cd.defenser = &MTGCardInstance::NoCard;
                    }
                    else
                    {
                        cd.defenser = &MTGCardInstance::AnyCard;
                    }
                }
                //Tapped, untapped
                else if (attribute.find("tapped") != string::npos)
                {
                    if (minus)
                    {
                        cd.unsecureSetTapped(-1);
                    }
                    else
                    {
                        cd.unsecureSetTapped(1);
                    }
                    //Token
                }
                else if (attribute.find("token") != string::npos)
                {
                    if (minus)
                    {
                        cd.isToken = -1;
                    }
                    else
                    {
                        cd.isToken = 1;
                    }
                    //put in its zone this turn
                }
                else if (attribute.find("fresh") != string::npos)
                {
                    if (minus)
                    {
                        cd.unsecuresetfresh(-1);
                    }
                    else
                    {
                        cd.unsecuresetfresh(1);
                    }
                }
                //creature is a level up creature
                else if (attribute.find("leveler") != string::npos)
                {
                    if (minus)
                    {
                        cd.isLeveler = -1;
                    }
                    else
                    {
                        cd.isLeveler = 1;
                    }
                }
                //creature is enchanted
                else if (attribute.find("enchanted") != string::npos)
                {
                    if (minus)
                    {
                        cd.CDenchanted = -1;
                    }
                    else
                    {
                        cd.CDenchanted = 1;
                    }
                }
                //creature was damaged
                else if (attribute.find("damaged") != string::npos)
                {
                    if (minus)
                    {
                        cd.CDdamaged = -1;
                    }
                    else
                    {
                        cd.CDdamaged = 1;
                    }
                }
                //creature dealt damage to opponent
                else if (attribute.find("opponentdamager") != string::npos)
                {
                    if (minus)
                    {
                        cd.CDopponentDamaged = -1;
                    }
                    else
                    {
                        cd.CDopponentDamaged = 1;
                    }
                }
                //creature dealt damage to controller
                else if (attribute.find("controllerdamager") != string::npos)
                {
                    if (minus)
                    {
                        cd.CDcontrollerDamaged = -1;
                    }
                    else
                    {
                        cd.CDcontrollerDamaged = 1;
                    }
                }
                else if (attribute.find("multicolor") != string::npos)
                {
                    //card is multicolored?
                    if (minus)
                    {
                        cd.setisMultiColored(-1);
                    }
                    else
                    {
                        cd.setisMultiColored(1);
                    }

                }
                else if (attribute.find("power") != string::npos)
                {
                    //Power restrictions
                    cd.setPower(comparisonCriterion);
                    cd.powerComparisonMode = comparisonMode;
                    //Toughness restrictions
                }
                else if (attribute.find("toughness") != string::npos)
                {
                    cd.setToughness(comparisonCriterion);
                    cd.toughnessComparisonMode = comparisonMode;
                    //Manacost restrictions
                }
                else if (attribute.find("manacost") != string::npos)
                {
                    cd.convertedManacost = comparisonCriterion;
                    cd.manacostComparisonMode = comparisonMode;
                    //Counter Restrictions
                }
                
                else
                {
                    int attributefound = 0;
                    ////Colors - remove Artifact and Land from the loop
                    
                    for (int cid = 1; cid < Constants::NB_Colors - 1; cid++)
                    { 
                        if (attribute.find(Constants::MTGColorStrings[cid]) != string::npos)
                        {
                            attributefound = 1;
                            if (minus)
                                cd.SetExclusionColor(cid);
                            else
                                cd.setColor(cid);
                        }
                    }
                    if (!attributefound)
                    {
                        //Abilities
                        for (int j = 0; j < Constants::NB_BASIC_ABILITIES; j++)
                        {
                            if (attribute.find(Constants::MTGBasicAbilities[j]) != string::npos)
                            {
                                attributefound = 1;
                                if (minus)
                                    cd.mAbilityExclusions.set(j);
                                else
                                    cd.basicAbilities.set(j);
                            }
                        }
                    }

                    if (!attributefound)
                    {
                        //Subtypes
                        if (minus)
                        {
                            cd.setNegativeSubtype(attribute);
                        }
                        else
                        {
                            cd.setSubtype(attribute);
                        }
                    }
                }
            }
            if (nbminuses)
                cd.mode = CD_AND;
            filter = filter.substr(0, found);
        }
        else
        {
            
            found = filter.find(",");
            
            if (found != string::npos)
            {
                cd.mode = CD_OR;
                typeName = filter.substr(0, found);
                filter = filter.substr(found + 1);
            }
            else
            {
                typeName = filter;
                filter = "";
            }
            
             cd.setSubtype(typeName);
        }

        
     } 
     if(cd.match(card)) 
         return true;
    return false;
    
}

void CardGui::RenderCountersBig(MTGCard * mtgcard, const Pos& pos, int drawMode)
{
    MTGCardInstance * card = dynamic_cast<MTGCardInstance*> (mtgcard);
    if (!card)
        return;

    if (!card->counters)
        return;
    if (!card->counters->mCount)
        return;

    // Write Named Counters
    WFont * font = WResourceManager::Instance()->GetWFont(Fonts::MAGIC_FONT);
    font->SetColor(ARGB((int)pos.actA, 0, 0, 0));
    font->SetScale(kWidthScaleFactor * pos.actZ);

    unsigned i = 0; 
    if (drawMode == DrawMode::kText)
    {
        std::vector<string> txt = card->data->getFormattedText();
        i = txt.size() + 1;
    }
    
    for (size_t t = 0; t < card->counters->counters.size(); t++)
    {
        Counter * c = card->counters->counters[t];

        if (!c || c->nb <= 0)
            continue;

        char buf[512];
        bool renderText = true;
        string gfx = "";
        //TODO cache the gfx fetch results?
        if (c->name.size()) 
        {
            if (c->nb < 6) //we only render a counter's specific quad if there are 5 counters of this type or less. Otherwise we will use the generic one
            {
                if (counterGraphics.find(c->name) == counterGraphics.end())
                {
                    string gfxRelativeName = "counters/";
                    gfxRelativeName.append(c->name);
                    gfxRelativeName.append(".png");
                    string _gfx = WResourceManager::Instance()->graphicsFile(gfxRelativeName);
                    if (!fileExists(_gfx.c_str()))
                        _gfx = "";
                    counterGraphics[c->name] = _gfx;
                }
                gfx = counterGraphics[c->name];
                if (gfx.size())
                    renderText = false;
            }

            if (renderText)
            {
                std::string s = c->name;
                s[0] = toupper(s[0]);
                sprintf(buf, _("%s: %i").c_str(), s.c_str(), c->nb);
            }
        }
        else
        {
            sprintf(buf, _("%s%i/%s%i").c_str(), ((c->power > 0) ? "+": ""), c->power * c->nb, ((c->toughness > 0) ? "+": ""),c->toughness* c->nb);
        }

        if (!gfx.size())
        {
            gfx = "counters/default.png";
        }
        
        float x = pos.actX + (22 - BigWidth / 2) * pos.actZ;
        float y =  pos.actY + (-BigHeight / 2 + 80 + 11 * i + 21 * t) * pos.actZ;
        if (y > pos.actY + 105) 
        {
           y =  (-BigHeight / 2 + 80 + 11 * i) * pos.actZ + (y - 105 - 21);
           x +=  (BigWidth / 2) * pos.actZ;
        }

        if (gfx.size())
        {
            JQuadPtr q = WResourceManager::Instance()->RetrieveTempQuad(gfx);

            if (q.get() && q->mTex)
            {
                float scale = 20.f / q->mHeight;
                if (renderText)
                {
                    float scaleX = (font->GetStringWidth(buf) + 20) / q->mWidth;
                    JRenderer::GetInstance()->RenderQuad(q.get(), x, y, 0, scaleX, scale);
                }
                else
                {
                    for (int j = 0; j < c->nb; ++j)
                    {
                        JRenderer::GetInstance()->RenderQuad(q.get(), x + (scale * q->mWidth * j), y, 0, scale, scale);
                    }
                }
            }
        }

        if (renderText)
        {
            font->SetColor(ARGB(255,0,0,0));
            font->DrawString(buf, x + 5, y + 5);
        }
    }
    
}

MTGCardInstance* CardView::getCard()
{
    return card;
}

TransientCardView::TransientCardView(MTGCardInstance* card, float x, float y)
    : CardGui(card, x, y)
{
}

TransientCardView::TransientCardView(MTGCardInstance* card, const Pos& ref)
    : CardGui(card, ref)
{
}
;

ostream& CardView::toString(ostream& out) const
{
    return (CardGui::toString(out) << " : CardView ::: card : " << card << ";  actX,actY : " << actX << "," << actY << "; t : "
        << t << " ; actT : " << actT);
}
ostream& CardGui::toString(ostream& out) const
{
    return (out << "CardGui ::: x,y " << x << "," << y);
}


SimpleCardEffectRotate::SimpleCardEffectRotate(float rotation): mRotation(rotation)
{
}
    
void SimpleCardEffectRotate::doEffect(Pos * card)
{
    card->t = mRotation;
}

void SimpleCardEffectRotate::undoEffect(Pos * card)
{
    card->t = 0;
}

SimpleCardEffectMask::SimpleCardEffectMask(PIXEL_TYPE mask): mMask(mask)
{
}
    
void SimpleCardEffectMask::doEffect(Pos * card)
{
    card->mask = mMask;
}

void SimpleCardEffectMask::undoEffect(Pos * card)
{
    card->mask = 0;
}

