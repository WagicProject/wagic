#ifndef _MTGGUIPLAY_H_
#define _MTGGUIPLAY_H_

#define MAX_ATTACKERS 20

#include "PlayGuiObjectController.h"
#include "Player.h"

class Player;
class GameObserver;
class CardGui;

class MTGGuiPlay: public PlayGuiObjectController {
 protected:
  GameObserver* game;
  int offset;
  Player * currentPlayer;
  MTGCardInstance * cardsGrid[SCREEN_WIDTH/5][SCREEN_HEIGHT/5];
  int nb_creatures[2];
  int nb_spells[2];
  int nb_lands[2];
  int cards_x_limit;

  JQuad * phaseIcons[24];
  JQuad * mGlitter;
  int mGlitterAlpha;
  float mGlitterX, mGlitterY;
  JTexture * mPhaseBarTexture;
  JQuad * mIcons[7];
  JTexture * mBgTex;
  JQuad * mBg;

  JTexture * mBgTex2;
  JQuad * mBg2;
  int alphaBg[4];
  void RenderPhaseBar();
  void RenderPlayerInfo(int player);
  JLBFont* mFont;

  void AddPlayersGuiInfo();
  void initCardsDisplay();
  void setCardPosition(CardGui * cardg, int player, int playerTurn, int spellMode);
  void setTargettingCardPosition(CardGui * cardg, int player, int playerTurn);
  void adjustCardPosition(CardGui * cardg);
 public:
  CardGui * getByCard(MTGCardInstance * card);
  MTGGuiPlay(GameObserver * game);
  ~MTGGuiPlay();
  void Update(float dt);
  bool CheckUserInput(u32 key);
  virtual void Render();
  void forceUpdateCards();
  void updateCards();
  int receiveEventPlus(WEvent * e);
};



#endif
