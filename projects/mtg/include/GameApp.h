/*
 *  Wagic, The Homebrew ?! is licensed under the BSD license
 *  See LICENSE in the Folder's root
 *  http://wololo.net/wagic/
 */

#ifndef _GAMEAPP_H_
#define _GAMEAPP_H_

#include <JApp.h>
#include <JGE.h>
#include <JSprite.h>
#include <JLBFont.h>
#include <hge/hgeparticle.h>
#include "WResourceManager.h"

#include "GameState.h"

#include "MTGDeck.h"
#include "MTGCard.h"
#include "MTGGameZones.h"

#include "CardEffect.h"
#ifdef NETWORK_SUPPORT
#include "JNetwork.h"
#endif //NETWORK_SUPPORT


class Rules;
enum
{
    PLAYER_TYPE_CPU = 0,
    PLAYER_TYPE_HUMAN = 1,
    PLAYER_TYPE_TESTSUITE = 2,
    PLAYER_TYPE_CPU_TEST = 3,
#ifdef NETWORK_SUPPORT
    PLAYER_TYPE_REMOTE = 4
#endif //NETWORK_SUPPORT
};

class MTGAllCards;
class TransitionBase;

class GameApp: public JApp
{

private:
#ifdef DEBUG
    int nbUpdates;
    float totalFPS;
#endif
    bool mShowDebugInfo;
    int mScreenShotCount;

    GameState* mCurrentState;
    GameState* mNextState;
    GameState* mGameStates[GAME_STATE_MAX];
public:

    GameType gameType;
    Rules * rules;
    CardEffect *effect;
#ifdef NETWORK_SUPPORT
    JNetwork* mpNetwork;
#endif //NETWORK_SUPPORT

    GameApp();
    virtual ~GameApp();

    virtual void Create();
    virtual void Destroy();
    virtual void Update();
    virtual void Render();
    virtual void Pause();
    virtual void Resume();

    virtual void OnScroll(int inXVelocity, int inYVelocity);

    void LoadGameStates();
    void SetNextState(int state);
    void SetCurrentState(GameState * state);
    void DoTransition(int trans, int tostate, float dur = -1, bool animonly = false);
    void DoAnimation(int trans, float dur = -1);
    static hgeParticleSystem * Particles[6];
    static bool HasMusic;
    static string systemError;
    static JMusic* music;
    static string currentMusicFile;
    static void playMusic(string filename = "", bool loop = true);
    static void stopMusic();
    static int players[2];

};

extern vector<JQuadPtr> manaIcons;

#endif
