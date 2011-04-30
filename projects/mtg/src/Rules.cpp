#include "PrecompiledHeader.h"

#include "Rules.h"
#include "MTGDefinitions.h"
#include "ManaCost.h"
#include "Player.h"
#include "AIMomirPlayer.h"

#include "MTGGameZones.h"
#include "MTGAbility.h"
#include "AllAbilities.h"
#include "DeckManager.h"
#include "AIPlayer.h"
#include <JLogger.h>


vector<Rules *> Rules::RulesList = vector<Rules *>();

//Sorting by dissplayName
struct RulesMenuCmp{
	bool operator()(const Rules * a,const Rules * b) const{
        return a->displayName < b->displayName;
	}
} RulesMenuCmp_;

Rules * Rules::getRulesByFilename(string _filename)
{
    for (size_t i = 0; i < RulesList.size(); ++i)
    {
        if (RulesList[i]->filename == _filename)
            return RulesList[i];
    }
    return NULL;
}

int Rules::loadAllRules()
{
    DIR *dip = opendir(JGE_GET_RES("rules").c_str());
    struct dirent *dit;

    while ((dit = readdir(dip)))
    {
        Rules * rules = NEW Rules();
        if (rules->load(dit->d_name))
        {
            RulesList.push_back(rules);
        }
        else
        {
            SAFE_DELETE(rules);
        }
    }
    //Kind of a hack here, we sort Rules alphabetically because it turns out to be matching
    // The historical order of Game modes: Classic, Momir Basic, Random 1, Random 2, Story
    std::sort(RulesList.begin(),RulesList.end(),RulesMenuCmp_);
    closedir(dip);
    return 1;
}

void Rules::unloadAllRules()
{
    for (size_t i = 0; i < RulesList.size(); ++i)
    {
        SAFE_DELETE(RulesList[i]);
    }
    RulesList.clear();
}

int Rules::getMTGId(string cardName)
{
    int cardnb = atoi(cardName.c_str());
    if (cardnb) return cardnb;
    if (cardName.compare("*") == 0) return -1; //Any card
    MTGCard * card = MTGCollection()->getCardByName(cardName);
    if (card) return card->getMTGId();
    DebugTrace("RULES: Can't find card:" << cardName.c_str());
    return 0;
}

MTGCardInstance * Rules::getCardByMTGId(int mtgid)
{
    GameObserver * g = GameObserver::GetInstance();
    for (int i = 0; i < 2; i++)
    {
        Player * p = g->players[i];
        MTGGameZone * zones[] = { p->game->library, p->game->hand, p->game->inPlay, p->game->graveyard };
        for (int j = 0; j < 4; j++)
        {
            MTGGameZone * zone = zones[j];
            for (int k = 0; k < zone->nb_cards; k++)
            {
                MTGCardInstance * card = zone->cards[k];
                if (!card) return NULL;
                if (card->getMTGId() == mtgid) return card;
            }
        }
    }
    return NULL;
}

RulesPlayerData::RulesPlayerData()
{
    life = 20;
    poisonCount = 0;
    damageCount = 0;
    preventable = 0;
    manapool = NEW ManaCost();
    avatar = "";
}

RulesPlayerData::~RulesPlayerData()
{
    SAFE_DELETE(manapool);
}

RulesPlayerZone::RulesPlayerZone()
{
}

void RulesPlayerZone::add(int cardId)
{
    cards.push_back(cardId);
}

RulesState::RulesState()
{
    phase = Constants::MTG_PHASE_FIRSTMAIN;
    player = 0;
}

void RulesState::parsePlayerState(int playerId, string s)
{
    size_t limiter = s.find("=");
    if (limiter == string::npos) limiter = s.find(":");
    string areaS;
    int area;
    if (limiter != string::npos)
    {
        areaS = s.substr(0, limiter);
        if (areaS.compare("graveyard") == 0)
        {
            area = 0;
        }
        else if (areaS.compare("library") == 0)
        {
            area = 1;
        }
        else if (areaS.compare("hand") == 0)
        {
            area = 2;
        }
        else if (areaS.compare("inplay") == 0 || areaS.compare("battlefield") == 0)
        {
            area = 3;
        }
        else if (areaS.compare("life") == 0)
        {
            playerData[playerId].life = atoi((s.substr(limiter + 1)).c_str());
            return;
        }
        else if (areaS.compare("poisoncount") == 0)
        {
            playerData[playerId].poisonCount = atoi((s.substr(limiter + 1)).c_str());
            return;
        }
        else if (areaS.compare("damagecount") == 0)
        {
            playerData[playerId].damageCount = atoi((s.substr(limiter + 1)).c_str());
            return;
        }
        else if (areaS.compare("preventable") == 0)
        {
            playerData[playerId].preventable = atoi((s.substr(limiter + 1)).c_str());
            return;
        }
        else if (areaS.compare("avatar") == 0)
        {
            playerData[playerId].avatar = s.substr(limiter + 1);
            return;
        }
        else if (areaS.compare("manapool") == 0)
        {
            SAFE_DELETE(playerData[playerId].manapool);
            playerData[playerId].manapool = ManaCost::parseManaCost(s.substr(limiter + 1));
            return;
        }
        else if (areaS.compare("customphasering") == 0)
        {
            playerData[playerId].phaseRing = s.substr(limiter + 1);
            return;
        }
        else if (areaS.compare("auto") == 0)
        {
            playerData[playerId].extraRules.push_back(s.substr(limiter + 1));
            return;
        }
        else
        {
            return; // ERROR
        }
        s = s.substr(limiter + 1);
        while (s.size())
        {
            unsigned int value;
            limiter = s.find(",");
            if (limiter != string::npos)
            {
                value = Rules::getMTGId(s.substr(0, limiter));
                s = s.substr(limiter + 1);
            }
            else
            {
                value = Rules::getMTGId(s);
                s = "";
            }
            if (value) playerData[playerId].zones[area].add(value);
        }
    }
    else
    {
        //ERROR
    }
}

void Rules::addExtraRules()
{
    GameObserver * g = GameObserver::GetInstance();

    int id = g->mLayers->actionLayer()->getMaxId();
    for (int i = 0; i < 2; ++i)
    {
        Player * p = g->players[i];
        //Trick so that the abilities don't die;
        MTGCardInstance::ExtraRules[i].currentZone = p->game->inPlay;
        MTGCardInstance::ExtraRules[i].lastController = p;
        for (size_t j = 0; j < initState.playerData[i].extraRules.size(); ++j)
        {
            AbilityFactory af;
            MTGPlayerCards * hand = NULL;
            int handsize = 7;
            int difficultyRating = 0;
            int Optimizedhandcheat = options[Options::OPTIMIZE_HAND].number;
            MTGAbility * a = af.parseMagicLine(initState.playerData[i].extraRules[j], id++, NULL, &MTGCardInstance::ExtraRules[i]);
            if (p->playMode != Player::MODE_TEST_SUITE && g->mRules->gamemode != GAME_TYPE_MOMIR && g->mRules->gamemode
                != GAME_TYPE_RANDOM1 && g->mRules->gamemode != GAME_TYPE_RANDOM2 && g->mRules->gamemode
                != GAME_TYPE_STORY && 
                g->mRules->gamemode != GAME_TYPE_DEMO && (!g->players[0] == PLAYER_TYPE_CPU && !g->players[1] == PLAYER_TYPE_CPU)
#ifdef NETWORK_SUPPORT
                && !g->players[1] == PLAYER_TYPE_REMOTE
#endif //NETWORK_SUPPORT
                    )//keep this out of momir and other game modes.
            {
                difficultyRating = DeckManager::getDifficultyRating(g->players[0], g->players[1]);
            }

            if (a)
            {
                if (a->oneShot)
                {
                    if (((p->isAI() && p->playMode
                        != Player::MODE_AI && p->opponent()->playMode
                        != Player::MODE_AI)||( !p->isAI() && Optimizedhandcheat)) && a->aType == MTGAbility::STANDARD_DRAW &&
                        difficultyRating != HARD && p->playMode
                        != Player::MODE_TEST_SUITE && g->mRules->gamemode != GAME_TYPE_MOMIR && g->mRules->gamemode
                        != GAME_TYPE_RANDOM1 && g->mRules->gamemode != GAME_TYPE_RANDOM2 && g->mRules->gamemode
                        != GAME_TYPE_STORY)//stupid protections to keep this out of mimor and other game modes.
                    {
                        handsize = ((AADrawer *)a)->getNumCards();
                        if(difficultyRating == EASY)
                        {
                            ((AIPlayer *) p)->forceBestAbilityUse = true;
                            ((AIPlayer *) p)->agressivity += 100;
                            hand->OptimizedHand(p,handsize, 3, 1, 3);//easy decks get a major boost, open hand is 2lands,1 creature under 3 mana,3spells under 3 mana.
                        }
                        else if (difficultyRating == NORMAL)
                        {
                            hand->OptimizedHand(p,handsize, 1, 0, 2);//give the Ai deck a tiny boost by giving it 1 land and 2 spells under 3 manacost.
                        }
                        else
                        {
                            hand->OptimizedHand(p,handsize, 3, 1, 3);//no rating fall out case.
                        }
                    }
                    else
                    {//resolve normally if the deck is listed as hard.
                        a->resolve();
                    }
                    delete (a);
                }
                else
                {
                    a->addToGame();
                }

            }
        }
    }

    for (size_t j = 0; j < extraRules.size(); ++j)
    {
        AbilityFactory af;
        MTGAbility * a = af.parseMagicLine(extraRules[j], id++, NULL, &MTGCardInstance::ExtraRules[0]);
        if (a)
        {
            if (a->oneShot)
            {
                a->resolve();
                delete (a);
            }
            else
            {
                a->addToGame();
            }
        }
    }

}

Player * Rules::loadPlayerMomir(int isAI)
{
    string deckFileSmall = "momir";
    char empty[] = "";

    MTGDeck * tempDeck = NEW MTGDeck(MTGCollection()); //Autogenerate a momir deck. Leave the "momir.txt" bits below for stats.
    tempDeck->addRandomCards(12, 0, 0, Constants::RARITY_L, "Forest");
    tempDeck->addRandomCards(12, 0, 0, Constants::RARITY_L, "Plains");
    tempDeck->addRandomCards(12, 0, 0, Constants::RARITY_L, "Swamp");
    tempDeck->addRandomCards(12, 0, 0, Constants::RARITY_L, "Mountain");
    tempDeck->addRandomCards(12, 0, 0, Constants::RARITY_L, "Island");

    Player *player = NULL;
    if (!isAI) // Human Player
        player = NEW HumanPlayer(options.profileFile("momir.txt", "", true).c_str(), deckFileSmall, tempDeck);
    else
        player = NEW AIMomirPlayer(options.profileFile("momir.txt", "", true).c_str(), deckFileSmall, empty, tempDeck);

    delete tempDeck;
    return player;
}

Player * Rules::loadPlayerRandom(int isAI, int mode)
{
    int color1 = 1 + WRand() % 5;
    int color2 = 1 + WRand() % 5;
    int color0 = Constants::MTG_COLOR_ARTIFACT;
    if (mode == GAME_TYPE_RANDOM1) color2 = color1;
    int colors[] = { color1, color2, color0 };
    int nbcolors = 3;

    string lands[] = { "forest", "forest", "island", "mountain", "swamp", "plains", "forest" };

    MTGDeck * tempDeck = NEW MTGDeck(MTGCollection());
    tempDeck->addRandomCards(9, 0, 0, -1, lands[color1].c_str());
    tempDeck->addRandomCards(9, 0, 0, -1, lands[color2].c_str());
    tempDeck->addRandomCards(1, 0, 0, 'U', "land");
    tempDeck->addRandomCards(1, 0, 0, 'R', "land");
    tempDeck->addRandomCards(12, 0, 0, -1, "creature", colors, nbcolors);
    tempDeck->addRandomCards(2, 0, 0, -1, "sorcery", colors, nbcolors);
    tempDeck->addRandomCards(2, 0, 0, -1, "enchantment", colors, nbcolors);
    tempDeck->addRandomCards(2, 0, 0, -1, "instant", colors, nbcolors);
    tempDeck->addRandomCards(2, 0, 0, -1, "artifact", colors, nbcolors);

    string deckFile = "random";
    string deckFileSmall = "random";

    Player *player = NULL;
    if (!isAI) // Human Player
        player = NEW HumanPlayer(deckFile, deckFileSmall, tempDeck);
    else
        player = NEW AIPlayerBaka(deckFile, deckFileSmall, "", tempDeck);

    delete tempDeck;
    return player;
}

Player * Rules::initPlayer(int playerId)
{
    GameObserver * g = GameObserver::GetInstance();
    Player * p = g->players[playerId];
    if (!p)
    {
        int isAI = 1;
        if (GameApp::players[playerId] == PLAYER_TYPE_HUMAN) isAI = 0;
        switch (gamemode)
        {
        case GAME_TYPE_MOMIR:
            return loadPlayerMomir(isAI);
        case GAME_TYPE_CLASSIC:
            return NULL; //Error for the time being
        case GAME_TYPE_RANDOM1:
            return loadPlayerRandom(isAI, GAME_TYPE_RANDOM1);
        case GAME_TYPE_RANDOM2:
            return loadPlayerRandom(isAI, GAME_TYPE_RANDOM2);
        }
    }
    p->phaseRing = initState.playerData[playerId].phaseRing;
    return p;
}

MTGDeck * Rules::buildDeck(int playerId)
{
    int nbcards = 0;
    MTGDeck * deck = NEW MTGDeck(MTGCollection());
    for (int j = 0; j < 4; j++)
    {
        for (size_t k = 0; k < initState.playerData[playerId].zones[j].cards.size(); k++)
        {
            int cardid = initState.playerData[playerId].zones[j].cards[k];
            deck->add(cardid);
            nbcards++;
        }
    }
    if (!nbcards)
    {
        delete (deck);
        return NULL;
    }
    return deck;
}

void Rules::initPlayers()
{
    GameObserver * g = GameObserver::GetInstance();
    for (int i = 0; i < 2; i++)
    {
        Player * p = initPlayer(i);
        g->players[i] = p;
        MTGDeck * deck = buildDeck(i);
        if (deck)
        {
            p->game->initDeck(deck);
            SAFE_DELETE(deck);
            p->game->setOwner(p);
        }
    }
}

void Rules::initGame()
{
    //Put the GameObserver in the initial state
    GameObserver * g = GameObserver::GetInstance();
    DebugTrace("RULES Init Game\n");

    //Set the current player/phase
    if (g->currentPlayer->playMode!= Player::MODE_TEST_SUITE &&  g->mRules->gamemode!= GAME_TYPE_STORY)
    {
        if(OptionWhosFirst::WHO_R == options[Options::FIRSTPLAYER].number)
            initState.player = WRand() % 2;
        if(OptionWhosFirst::WHO_O == options[Options::FIRSTPLAYER].number)
            initState.player = 1;
    }
    g->currentPlayer = g->players[initState.player];
    g->currentActionPlayer = g->currentPlayer;
    g->currentPlayerId = initState.player;
    g->phaseRing->goToPhase(0, g->currentPlayer, false);
    g->phaseRing->goToPhase(initState.phase, g->currentPlayer);
    g->currentGamePhase = initState.phase;

    for (int i = 0; i < 2; i++)
    {
        Player * p = g->players[i];
        p->life = initState.playerData[i].life;
        p->poisonCount = initState.playerData[i].poisonCount;
        p->damageCount = initState.playerData[i].damageCount;
        p->preventable = initState.playerData[i].preventable;
        p->getManaPool()->copy(initState.playerData[i].manapool);
        if (initState.playerData[i].avatar.size())
        {
            p->loadAvatar(initState.playerData[i].avatar);
        }
        MTGGameZone * playerZones[] = { p->game->graveyard, p->game->library, p->game->hand, p->game->inPlay };
        for (int j = 0; j < 4; j++)
        {
            MTGGameZone * zone = playerZones[j];
            for (size_t k = 0; k < initState.playerData[i].zones[j].cards.size(); k++)
            {
                MTGCardInstance * card = getCardByMTGId(initState.playerData[i].zones[j].cards[k]);
                if (card && zone != p->game->library)
                {
                    if (zone == p->game->inPlay)
                    {
                        MTGCardInstance * copy = p->game->putInZone(card, p->game->library, p->game->stack);
                        Spell * spell = NEW Spell(copy);
                        spell->resolve();
                        delete spell;
                    }
                    else
                    {
                        if (!p->game->library->hasCard(card))
                        {
                            LOG ("RULES ERROR, CARD NOT FOUND IN LIBRARY\n");
                        }
                        p->game->putInZone(card, p->game->library, zone);
                    }
                }
                else
                {
                    if (!card)
                    {
                        LOG ("RULES ERROR, card is NULL\n");
                    }
                }
            }
        }
    }
    addExtraRules();
DebugTrace("RULES Init Game Done !\n");
}

void RulesPlayerZone::cleanup()
{
    cards.clear();
}

void RulesPlayerData::cleanup()
{
    if (manapool) delete manapool;
    manapool = NULL;
    manapool = NEW ManaCost();
    for (int i = 0; i < 5; i++)
    {
        zones[i].cleanup();
    }
    life = 20;
    poisonCount = 0;
    damageCount = 0;
    preventable = 0;
}

void RulesState::cleanup()
{
    for (int i = 0; i < 2; i++)
    {
        playerData[i].cleanup();
    }
}

void Rules::cleanup()
{
    initState.cleanup();
}

Rules::Rules(string _bg)
{
    bg = _bg;
    unlockOption = INVALID_OPTION;
    hidden = false;
    filename = "";
}

bool Rules::canChooseDeck() 
{
    return (gamemode == GAME_TYPE_CLASSIC); 
}

int Rules::load(string _filename)
{
    //avoid non .txt files
    if (_filename.size() < 5 || (_filename.find(".txt") == string::npos && _filename.find(".TXT") == string::npos))
        return 0;

    if (!filename.size()) //this check is necessary because of the recursive calls (a fil loads other files)
        filename = _filename;
    char c_filename[4096];
    if (fileExists(_filename.c_str()))
    {
        sprintf(c_filename, "%s", _filename.c_str());
    }
    else
    {
        sprintf(c_filename, JGE_GET_RES("rules/%s").c_str(), _filename.c_str());
    }
    wagic::ifstream file(c_filename);
    std::string s;

    int state = PARSE_UNDEFINED;

    //  std::cout << std::endl << std::endl << "!!!" << file << std::endl << std::endl;
    if (!file) return 0;

    cleanup();
    while (std::getline(file, s))
    {
        if (!s.size()) continue;
        if (s[s.size() - 1] == '\r') s.erase(s.size() - 1); //Handle DOS files
        if (s[0] == '#') continue;
        string scopy = s;
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        if (s.find("include ") == 0)
        {
            load(s.substr(8));
            continue;
        }
        if (s.compare("[init]") == 0)
        {
            state = PARSE_INIT;
            continue;
        }
        if (s.compare("[players]") == 0)
        {
            state = PARSE_PLAYERS;
            continue;
        }
        if (s.compare("[player1]") == 0)
        {
            state = PARSE_PLAYER1;
            continue;
        }
        if (s.compare("[player2]") == 0)
        {
            state = PARSE_PLAYER2;
            continue;
        }

        switch (state)
        {
        case PARSE_UNDEFINED:
            if (s.find("name=") == 0)
            {
                displayName = scopy.substr(5);
            }
            else if (s.find("unlock=") == 0)
            {
                unlockOption = Options::getID(s.substr(7));
            }
            else if (s.find("hidden") == 0)
            {
                hidden = true;
            }
            break;
        case PARSE_INIT:
            if (s.find("auto=") == 0)
            {
                extraRules.push_back(s.substr(5));
            }
            else if (s.find("mode=") == 0)
            {
                gamemode = strToGameMode(s.substr(5));
            }
            else if (s.find("player=") == 0)
            {
                initState.player = atoi(s.substr(7).c_str()) - 1;
            }
            else
            {
                initState.phase = PhaseRing::phaseStrToInt(s);
            }
            break;
        case PARSE_PLAYER1:
            initState.parsePlayerState(0, s);
            break;
        case PARSE_PLAYER2:
            initState.parsePlayerState(1, s);
            break;
        case PARSE_PLAYERS:
            initState.parsePlayerState(0, s);
            initState.parsePlayerState(1, s);
            break;
        }
    }
    file.close();
    return 1;
}

int Rules::strToGameMode(string s)
{
    if (s.compare("momir") == 0) return GAME_TYPE_MOMIR;
    if (s.compare("random1") == 0) return GAME_TYPE_RANDOM1;
    if (s.compare("random2") == 0) return GAME_TYPE_RANDOM2;
    if (s.compare("story") == 0) return GAME_TYPE_STORY;
    return GAME_TYPE_CLASSIC;
}
