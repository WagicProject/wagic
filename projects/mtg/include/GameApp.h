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
#include "GameObserver.h"

/* Wagic versions */
#define WAGIC_VERSION_MAJOR     0
#define WAGIC_VERSION_MEDIUM    17
#define WAGIC_VERSION_MINOR     1

#define VERSION_DOT(a, b, c) a ##.## b ##.## c
#define VERSION(a, b, c) VERSION_DOT(a, b, c)
#define VERSION_TOSTRING(a) #a
#define VERSION_STRINGIFY(a) VERSION_TOSTRING(a)

#define WAGIC_VERSION   VERSION(WAGIC_VERSION_MAJOR, WAGIC_VERSION_MEDIUM, WAGIC_VERSION_MINOR)
#define WAGIC_VERSION_STRING   VERSION_STRINGIFY(WAGIC_VERSION)

class Rules;
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
    static PlayerType players[2];

};

extern vector<JQuadPtr> manaIcons;

#endif
