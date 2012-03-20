#include "PrecompiledHeader.h"

#include "Credits.h"
#include "GameApp.h"
#include "PlayerData.h"
#include "DeckStats.h"
#include "Translate.h"
#include "MTGDeck.h"
#include "GameObserver.h"
#include "GameStateShop.h"
#include "AIPlayer.h"


map <string, Unlockable *> Unlockable::unlockables;

Unlockable::Unlockable() 
{
}

void  Unlockable::setValue(string k , string v) 
{
    mValues[k] = v;
}

string Unlockable::getValue(string k) 
{
    return mValues[k];
}

bool Unlockable::isUnlocked() {
    string id = getValue("id");
    assert(id.size() > 0);
    return (options[id].number != 0);
}

bool Unlockable::tryToUnlock(GameObserver * game) {
    if (isUnlocked())
        return false;

    string conditions = getValue("unlock_condition");
    
    Player * p = game->players[0];
    if (p->isAI())
        return false;

    // We need a card belonging to the player in order to call parceCastRestrictions
    // The goal is usually to create objects such as targetChoosers, that are usually required
    // for protection, or determining the card's owner
    //Therefore here any card is ok
    MTGCardInstance * dummyCard = p->game->battlefield->nb_cards 
        ? p->game->battlefield->cards[0]
        :  p->game->hand->nb_cards 
            ?  p->game->hand->cards[0]
            : p->game->library->nb_cards
                ?  p->game->library->cards[0]
                : NULL;

    AbilityFactory af(game);
    int meetConditions = conditions.size() 
        ? dummyCard 
            ? af.parseCastRestrictions(dummyCard, p, conditions) 
            : 0
        : 1;

    if (!meetConditions)
        return false;

    // Unlock the award and return
    string id = getValue("id");
    assert(id.size() > 0);

	GameOptionAward* goa = (GameOptionAward*) &options[id]; 
    goa->giveAward();
    return true;
}

void Unlockable::load()
{
    std::string contents;
    if (! JFileSystem::GetInstance()->readIntoString("rules/awards.dat", contents))
        return;

    std::stringstream stream(contents);
    std::string s;

    Unlockable * current = NULL;
    while (std::getline(stream, s))
    {
        if (!s.size()) continue;
        if (s[s.size() - 1] == '\r') s.erase(s.size() - 1); //Handle DOS files
        if (!s.size()) continue;
        if (s[0] == '#') continue;

        if (s == "[award]")
        {
            current = NEW Unlockable();
            continue;
        }


        if (!current)
            continue;

        if (s == "[/award]")
        {
            string id = current->getValue("id");
            
            if (id.size())
                unlockables[id] = current;
            else 
                SAFE_DELETE(current);

            continue;
        }

        vector<string> keyValue = split(s,'=');
        if (keyValue.size() != 2)
            continue;

        current->setValue(keyValue[0], keyValue[1]);
    }
}

void Unlockable::Destroy()
{
    for (map<string, Unlockable *>::iterator it = unlockables.begin(); it != unlockables.end(); ++it) {
        SAFE_DELETE(it->second);
    }
    unlockables.clear();
}

CreditBonus::CreditBonus(int _value, string _text)
{
    value = _value;
    text = _text;
}

void CreditBonus::Render(float x, float y, WFont * font)
{
    char buffer[512];
    sprintf(buffer, "%s: %i", text.c_str(), value);
    font->DrawString(buffer, x, y);
}

Credits::Credits()
{
    unlocked = -1;
    p1 = NULL;
    p2 = NULL;
    observer = NULL;
}

Credits::~Credits()
{
    for (unsigned int i = 0; i < bonus.size(); ++i)
        if (bonus[i])
            delete bonus[i];
    bonus.clear();
}

void Credits::compute(GameObserver* g, GameApp * _app)
{
    if (!g->turn)
        return;
    p1 = g->players[0];
    p2 = g->players[1];
    observer = g;
    app = _app;
    showMsg = (WRand() % 3);

    //no credits when the AI plays :)
    if (p1->isAI())
        return;

    PlayerData * playerdata = NEW PlayerData(MTGCollection());
    if (p2->isAI() && g->didWin(p1))
    {
        gameLength = time(0) - g->startedAt;
        value = 400;
        if (app->gameType != GAME_TYPE_CLASSIC)
            value = 200;
        int difficulty = options[Options::DIFFICULTY].number;
        if (options[Options::DIFFICULTY_MODE_UNLOCKED].number && difficulty)
        {
            CreditBonus * b = NEW CreditBonus(100 * difficulty, _("Difficulty Bonus"));
            bonus.push_back(b);
        }

        if (p1->life == 1)
        {
            CreditBonus * b = NEW CreditBonus(111, _("'Live dangerously and you live right' Bonus"));
            bonus.push_back(b);
        }

        int diff = p1->life - p2->life;
        if (diff < 0)
            diff = 0;
        if (diff > 500)
            diff = 500;
        if (diff)
        {
            CreditBonus * b = NEW CreditBonus(diff, _("Life Delta Bonus"));
            bonus.push_back(b);
        }

        if (p1->game->library->nb_cards == 0)
        {
            CreditBonus * b = NEW CreditBonus(391, _("'Decree of Theophilus' Bonus"));
            bonus.push_back(b);
        }

        if ((p2->game->library->nb_cards == 0) && p1->game->library->nb_cards)
        {
            CreditBonus * b = NEW CreditBonus(p1->game->library->nb_cards * 3, _("Miller Bonus"));
            bonus.push_back(b);
        }

        if (g->turn < 15)
        {
            CreditBonus * b = NEW CreditBonus((20 - g->turn) * 17, _("'Fast and Furious' Bonus"));
            bonus.push_back(b);
        }

        GameOptionAward * goa = NULL;
        // <Tasks handling>
        vector<Task*> finishedTasks;
        playerdata->taskList->getDoneTasks(g, _app, &finishedTasks);

        char buffer[512];

        for (vector<Task*>::iterator it = finishedTasks.begin(); it != finishedTasks.end(); it++)
        {
            sprintf(buffer, _("Task: %s").c_str(), (*it)->getShortDesc().c_str());
            CreditBonus * b = NEW CreditBonus((*it)->getReward(), buffer);
            bonus.push_back(b);
            playerdata->taskList->removeTask(*it);
        }
        // </Tasks handling>

        if (unlocked == -1)
        {
            DeckStats * stats = DeckStats::GetInstance();
            stats->load(p1->GetCurrentDeckStatsFile());
            unlocked = isDifficultyUnlocked(stats);
            if (unlocked)
            {
                unlockedTextureName = "unlocked.png";
                goa = (GameOptionAward*) &options[Options::DIFFICULTY_MODE_UNLOCKED];
                goa->giveAward();
            }
            else
            {
                for (map<string, Unlockable *>::iterator it = Unlockable::unlockables.begin(); it !=  Unlockable::unlockables.end(); ++it) {
                    Unlockable * award = it->second;
                    if (award->tryToUnlock(g))
                    {
                        unlocked = 1;
                        unlockedString = award->getValue("unlock_text");
                        unlockedTextureName = award->getValue("unlock_img");
                        break;
                    }
                }
            }
            
            if (!unlocked)
            {
                if ((unlocked = isEvilTwinUnlocked()))
                {
                    unlockedTextureName = "eviltwin_unlocked.png";
                    goa = (GameOptionAward*) &options[Options::EVILTWIN_MODE_UNLOCKED];
                    goa->giveAward();
                }
                else if ((unlocked = isRandomDeckUnlocked()))
                {
                    unlockedTextureName = "randomdeck_unlocked.png";
                    goa = (GameOptionAward*) &options[Options::RANDOMDECK_MODE_UNLOCKED];
                    goa->giveAward();
                }
                else if ((unlocked = unlockRandomSet()))
                {
                    unlockedTextureName = "set_unlocked.png";
                    MTGSetInfo * si = setlist.getInfo(unlocked - 1);
                    if (si)
                        unlockedString = si->getName(); //Show the set's pretty name for unlocks.
                }
                else if ((unlocked = IsMoreAIDecksUnlocked(stats)))
                {
                    options[Options::AIDECKS_UNLOCKED].number += 10;
                    options.save();
                    unlockedTextureName = "ai_unlocked.png";
                }
            }

            if (unlocked && options[Options::SFXVOLUME].number > 0)
            {
                WResourceManager::Instance()->PlaySample("bonus.wav");
            }

        }

        vector<CreditBonus *>::iterator it;
        if (bonus.size())
        {
            CreditBonus * b = NEW CreditBonus(value, _("Victory"));
            bonus.insert(bonus.begin(), b);
            for (it = bonus.begin() + 1; it < bonus.end(); ++it)
                value += (*it)->value;
        }

        playerdata->credits += value;
        PriceList::updateKey();
        playerdata->taskList->passOneDay();
        if (playerdata->taskList->getTaskCount() < 6)
        {
            playerdata->taskList->addRandomTask();
            playerdata->taskList->addRandomTask();
        }

    }
    else
    {
        unlocked = 0;
        playerdata->taskList->passOneDay();
    }

    playerdata->save();
    SAFE_DELETE(playerdata);
}

JQuadPtr Credits::GetUnlockedQuad(string textureName)
{
    if (!textureName.size()) return JQuadPtr();

    JTexture * unlockedTex = WResourceManager::Instance()->RetrieveTexture(textureName);
    if (!unlockedTex) return JQuadPtr();

    return WResourceManager::Instance()->RetrieveQuad(
        unlockedTextureName,
        2.0f,
        2.0f, 
        static_cast<float>(unlockedTex->mWidth - 4),
        static_cast<float>(unlockedTex->mHeight - 4));
    
}

void Credits::Render()
{
    if (!p1)
        return;
    JRenderer * r = JRenderer::GetInstance();
    WFont * f = WResourceManager::Instance()->GetWFont(Fonts::MAIN_FONT);
    WFont * f2 = WResourceManager::Instance()->GetWFont(Fonts::MENU_FONT);
    WFont * f3 = WResourceManager::Instance()->GetWFont(Fonts::MAGIC_FONT);
    f->SetScale(1);
    f->SetColor(ARGB(255,255,255,255));
    f2->SetScale(1);
    f2->SetColor(ARGB(255,255,255,255));
    f3->SetScale(1);
    f3->SetColor(ARGB(255,255,255,255));
    char buffer[512];

    if (!observer->turn)
    {
        sprintf(buffer, "%s", _("Please check your deck (not enough cards?)").c_str());
    }
    else
    {
        if (!p1->isAI() && p2->isAI())
        {
            if (observer->didWin(p1))
            {
                sprintf(buffer, _("Congratulations! You earn %i credits").c_str(), value);
                JQuadPtr unlockedQuad = GetUnlockedQuad(unlockedTextureName);
                if (unlockedQuad)
                {
                    showMsg = 0;
                    r->RenderQuad(unlockedQuad.get(), 20, 20);
                }
                if (unlockedString.size())
                {
                    f2->DrawString(unlockedString.c_str(), SCREEN_WIDTH / 2, 80, JGETEXT_CENTER);
                }
            }
            else
            {
                sprintf(buffer, "%s", _("You have been defeated").c_str());
            }
        }
        else
        {
            int winner = 2;
            if (observer->didWin(p1))
            {
                winner = 1;
            }
            int p0life = p1->life;
            sprintf(buffer, _("Player %i wins (%i)").c_str(), winner, p0life);
        }
    }

    float y = 130;

    if (showMsg == 1)
        y = 50;
    vector<CreditBonus *>::iterator it;
    for (it = bonus.begin(); it < bonus.end(); ++it)
    {
        (*it)->Render(10, y, f3);
        y += 12;
    }
    f2->DrawString(buffer, 10, y);
    y += 15;

    //!!
    if (observer->didWin(p1) && this->gameLength != 0)
    {
        sprintf(buffer, _("Game length: %i turns (%i seconds)").c_str(), observer->turn, this->gameLength);
        f->DrawString(buffer, 10, y);
        y += 10;
        sprintf(buffer, _("Credits per minute: %i").c_str(), (int) (60 * value / this->gameLength));
        f->DrawString(buffer, 10, y);
        y += 10;
        showMsg = 0;
    }

    if (showMsg == 1)
    {
        f2->DrawString(_("There's more!").c_str(), 10, y + 15);
        f->DrawString(_("Mods, additional cards, updates and more at:").c_str(), 10, y + 30);
        f2->DrawString("-> http://wololo.net/wagic", 10, y + 42);
    }

}

int Credits::isDifficultyUnlocked(DeckStats * stats)
{
    if (options[Options::DIFFICULTY_MODE_UNLOCKED].number)
        return 0;
    int nbAIDecks = AIPlayer::getTotalAIDecks();

    int wins = 0;

    for (int i = 0; i < nbAIDecks; ++i) 
    {
        char aiSmallDeckName[512];
        sprintf(aiSmallDeckName, "ai_baka_deck%i", i+1);
        int percentVictories = stats->percentVictories(string(aiSmallDeckName));
        if (percentVictories >= 67)
            wins++;
        if (wins >= 10)
            return 1;
    }
    return 0;
}

int Credits::isEvilTwinUnlocked()
{
    if (options[Options::EVILTWIN_MODE_UNLOCKED].number)
        return 0;
    if (p1->game->inPlay->nb_cards && (p1->game->inPlay->nb_cards == p2->game->inPlay->nb_cards))
        return 1;
    return 0;
}

int Credits::isRandomDeckUnlocked()
{
    if (0 == options[Options::DIFFICULTY].number)
        return 0;
    if (options[Options::RANDOMDECK_MODE_UNLOCKED].number)
        return 0;
    if (p1->life >= 20)
        return 1;
    return 0;
}

int Credits::addCreditBonus(int value)
{
    PlayerData * playerdata = NEW PlayerData();
    playerdata->credits += value;
    playerdata->save();
    SAFE_DELETE(playerdata);
    return value;
}
/*
 * adds a Card to a deck
 * @param cardId id of the card
 * @param collection deck representing player's collection
 */
int Credits::addCardToCollection(int cardId, MTGDeck * collection)
{
    return collection->add(cardId);
}

/*
 * adds a Card to player's collection
 * prefer to call the above function if you want to add several cards, since saving is expensive
 */
int Credits::addCardToCollection(int cardId)
{
    PlayerData * playerdata = NEW PlayerData(MTGCollection());
    int result = addCardToCollection(cardId, playerdata->collection);
    playerdata->collection->save();
    return result;
}

int Credits::unlockSetByName(string name)
{
    int setId = setlist.findSet(name);
    if (setId < 0)
        return 0;

    GameOptionAward* goa = (GameOptionAward*) &options[Options::optionSet(setId)];
    goa->giveAward();
    options.save();
    return setId + 1; //We add 1 here to show success/failure. Be sure to subtract later.

}

int Credits::unlockRandomSet(bool force)
{
    int setId = WRand() % setlist.size();

    if (force)
    {
        int init = setId;
        bool found = false;
        do
        {
            if (1 != options[Options::optionSet(setId)].number)
                found = true;
            else
            {
                setId++;
                if (setId == setlist.size())
                    setId = 0;
            }
        } while (setId != init && !found);
    }

    if (1 == options[Options::optionSet(setId)].number)
        return 0;

    GameOptionAward* goa = (GameOptionAward*) &options[Options::optionSet(setId)];
    goa->giveAward();
    options.save();
    return setId + 1; //We add 1 here to show success/failure. Be sure to subtract later.
}


int Credits::IsMoreAIDecksUnlocked(DeckStats * stats) {
    int currentlyUnlocked = options[Options::AIDECKS_UNLOCKED].number;

    // Random rule: having played at least 1.2 times as much games as 
    // the number of currently unlocked decks in order to go through.
    if (stats->nbGames() < currentlyUnlocked * 1.2) return 0;

    if (AIPlayer::getTotalAIDecks() > currentlyUnlocked)
        return 1;

    return 0;
}
