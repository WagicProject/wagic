#ifndef NETWORKPLAYER_H
#define NETWORKPLAYER_H

#include "PrecompiledHeader.h"
#include "Player.h"
#include "JNetwork.h"

extern void RegisterNetworkPlayers();

class ProxyPlayer
{
protected:
    Player* mpPlayer;
    JNetwork* mpNetwork;
    static ProxyPlayer* mInstance;
public:
    ProxyPlayer(Player* pxPlayer, JNetwork* pxNetwork);
    static void Serialize(void*, stringstream& in, stringstream& out);
};


class RemotePlayer : public Player
{
protected:
    JNetwork* mpNetwork;
    static RemotePlayer* mInstance;
public:
    RemotePlayer(GameObserver* observer, JNetwork*);
    static void Deserialize(void*, stringstream& in, stringstream& out);
    bool isLoaded() {return game!=NULL;};
};

#endif // NETWORKPLAYER_H
