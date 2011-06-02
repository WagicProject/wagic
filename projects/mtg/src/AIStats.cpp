#include "PrecompiledHeader.h"

#include "AIStats.h"
#include "GameObserver.h"
#include "Player.h"
#include "MTGCardInstance.h"
#include "WEvent.h"
#include "AllAbilities.h"
//TODO:better comments this is too cryptic to work on by anyone but original coder.
bool compare_aistats(AIStat * first, AIStat * second)
{
    float damage1 = static_cast<float> (first->value / first->occurences);
    float damage2 = static_cast<float> (second->value / second->occurences);
    return (damage1 > damage2);
}

AIStats::AIStats(Player * _player, char * _filename)
{
    filename = _filename;
    load(_filename);
    player = _player;
}

AIStats::~AIStats()
{
    list<AIStat *>::iterator it;
    for (it = stats.begin(); it != stats.end(); it++)
    {
        AIStat * stat = *it;
        delete stat;
    }
}

void AIStats::updateStatsCard(MTGCardInstance * cardInstance, Damage * damage, float multiplier)
{
    MTGCard * card = cardInstance->model;
    if (!card)
        return; //card can be null because some special cardInstances (such as ExtraRules) don't have a "model"

    AIStat * stat = find(card);
    if (!stat)
    {
        stat = NEW AIStat(card->getMTGId(), 0, 1, 0);
        stats.push_back(stat);
    }
    if (damage->target == player)
    {
        stat->value += static_cast<int>(multiplier * STATS_PLAYER_MULTIPLIER * damage->damage);
    }
    else if (damage->target->type_as_damageable == DAMAGEABLE_MTGCARDINSTANCE)
    {
        MTGCardInstance * target = (MTGCardInstance *) damage->target;
        if (target->controller() == player && !target->isInPlay())
        {
            //One of my creatures got lethal damage...
            stat->value += static_cast<int>(multiplier * STATS_CREATURE_MULTIPLIER * damage->damage);
        }
    }
}

int AIStats::receiveEvent(WEvent * event)
{
    WEventDamage * e = dynamic_cast<WEventDamage *> (event);
    if (!e)
        return 0; //we take only Damage events into accountright now
    Damage * damage = e->damage;
    MTGGameZone * opponentZone = player->opponent()->game->inPlay;

    MTGCardInstance * card = damage->source;
    updateStatsCard(card, damage);

    //Auras on damage source can be the cause
    for (int i = 0; i < opponentZone->nb_cards; ++i)
    {
        MTGCardInstance * aura = opponentZone->cards[i];
        if (aura->target == card)
        {
            updateStatsCard(aura, damage, STATS_AURA_MULTIPLIER);
        }
    }

    GameObserver * g = GameObserver::GetInstance();
    //Lords
    map<MTGCardInstance *, int> lords;
    for (size_t i = 1; i < g->mLayers->actionLayer()->mObjects.size(); i++)
    { //0 is not a mtgability...hackish
        MTGAbility * a = ((MTGAbility *) g->mLayers->actionLayer()->mObjects[i]);
        if (ALord * al = dynamic_cast<ALord*>(a))
        {
            if (al->cards.find(card) != al->cards.end() && opponentZone->hasCard(al->source))
            {
                lords[al->source] = 1;
            }
        }
    }
    if (size_t nb = lords.size())
    {
        for (map<MTGCardInstance *, int>::iterator it = lords.begin(); it != lords.end(); ++it)
        {
            updateStatsCard(it->first, damage, STATS_LORD_MULTIPLIER / nb);
        }
    }

    stats.sort(compare_aistats); //this could be slow, if it is, let's run it only at the end of the turn
    return 1; //is this meant to return 0 or 1?
}
//TODO:what does this do?
bool AIStats::isInTop(MTGCardInstance * card, unsigned int max, bool tooSmallCountsForTrue)
{
	//return true; 
	//uncomment the above return to make Ai always multiblock your creatures.
    if (stats.size() < max)
        return tooSmallCountsForTrue;
    unsigned int n = 0;
    MTGCard * source = card->model;
    int id = source->getMTGId();
    list<AIStat *>::iterator it;
    for (it = stats.begin(); it != stats.end(); it++)
    {
        if (n >= max)
            return false;
        AIStat * stat = *it;
        if (stat->source == id)
        {
            if ((stat->value + card->DangerRanking()) >= 3)
                return true;
            return false;
        }
        n++;
    }
    return false;
}

AIStat * AIStats::find(MTGCard * source)
{
    int id = source->getMTGId();
    list<AIStat *>::iterator it;
    for (it = stats.begin(); it != stats.end(); it++)
    {
        AIStat * stat = *it;
        if (stat->source == id)
            return stat;
    }
    return NULL;
}

void AIStats::load(char * filename)
{
    wagic::ifstream file(filename);
    std::string s;

    if (file)
    {
        while (std::getline(file, s))
        {
            int cardid = atoi(s.c_str());
            std::getline(file, s);
            int value = atoi(s.c_str());
            std::getline(file, s);
            bool direct = atoi(s.c_str()) > 0;
            AIStat * stat = NEW AIStat(cardid, value, 1, direct);
            stats.push_back(stat);
        }
        file.close();
    }
    else
    {
        //TODO Error management
    }
}
void AIStats::save()
{
    std::ofstream file(filename.c_str());
    char writer[128];
    if (file)
    {
        list<AIStat *>::iterator it;
        for (it = stats.begin(); it != stats.end(); it++)
        {
            AIStat * stat = *it;
            if (stat->value > 0)
            {
                sprintf(writer, "%i\n%i\n%i\n", stat->source, stat->value / 2, stat->direct);
                file << writer;
            }
        }
        file.close();
    }

}

void AIStats::Render()
{
    GameObserver * g = GameObserver::GetInstance();
    float x0 = 10;
    if (player == g->players[1])
        x0 = 280;
    JRenderer::GetInstance()->FillRoundRect(x0, 10, 200, 180, 5, ARGB(50,0,0,0));

    WFont * f = WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);
    int i = 0;
    char buffer[512];
    list<AIStat *>::iterator it;
    for (it = stats.begin(); it != stats.end(); ++it)
    {
        if (i > 10)
            break;
        AIStat * stat = *it;
        if (stat->value > 0)
        {
            MTGCard * card = MTGCollection()->getCardById(stat->source);
            if (card)
            {
                sprintf(buffer, "%s %i", card->data->getName().c_str(), stat->value);
                f->DrawString(buffer, x0 + 5, 10 + 16 * (float) i);
                i++;
            }
        }
    }
}
