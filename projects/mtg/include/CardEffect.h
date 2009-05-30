#ifndef _CARDEFFECT_H_
#define _CARDEFFECT_H_

#include <JGE.h>
#include "Effects.h"

class CardEffect : public Effect
{
 public:
  CardEffect();
  ~CardEffect();
 private:
  static PIXEL_TYPE surface[MTG_IMAGE_WIDTH*MTG_IMAGE_HEIGHT];
  unsigned char sineTable1[256];
  unsigned char sineTable2[256];
  PIXEL_TYPE palette[256];
  JTexture * backTexture;
  JTexture * backThumbTexture;

 public:
  void UpdateSmall(float dt);
  void UpdateBig(float dt);
};


#endif // _CARDEFFECT_H_
