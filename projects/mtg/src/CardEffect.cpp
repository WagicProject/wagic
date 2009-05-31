#include "../include/GameApp.h"
#include "../include/MTGCard.h"
#include "../include/GameOptions.h"
#include "../include/CardEffect.h"

PIXEL_TYPE CardEffect::surface[] = {};

CardEffect::CardEffect()
{
  backTexture = GameApp::CommonRes->GetQuad("back")->mTex;
  backThumbTexture = GameApp::CommonRes->GetQuad("back_thumb")->mTex;

  for (int i = sizeof(palette)/sizeof(palette[0]) - 1; i >= 0; --i)
    {
      sineTable1[i] = 127 + 127 * sinf(2*M_PI*((float)i)/256.0) * sinf(2*M_PI*((float)i)/256.0);
      sineTable2[i] = 127 + 127 * sinf(M_PI * sinf(2*M_PI*((float)i)/256.0));

      palette[i] = ARGB(((unsigned char)(191 + 64 * (sinf(M_PI*sinf(2*M_PI*((float)i)/256.0)/2)))),
			0,
			(unsigned char)(200 * (0.5 + sinf(2*M_PI*((float)i)/256.0)/2) * (0.5 + sinf(2*M_PI*((float)i)/256.0)/2)),
      (unsigned char)(200 * (0.5 + sinf(2*M_PI*((float)i)/256.0)/2) ),

			);
    }
}

CardEffect::~CardEffect()
{

}

void CardEffect::UpdateSmall(float dt)
{
  if (!GameOptions::GetInstance()->values[OPTIONS_PLASMAEFFECT].getIntValue()) return;
  static float t = 0;
  t += 3*dt;
  unsigned char c = (unsigned char)(5*t);
  for (int j = MTG_MINIIMAGE_HEIGHT - 1; j >= 0; --j)
    for (int i = MTG_MINIIMAGE_WIDTH - 1; i >= 0; --i)
      {
	unsigned int r = c + sineTable1[0xFF & (c + i*2)] + sineTable1[0xFF & ((j+i)*sineTable1[c]/128)];
	r %= 255;
	surface[MTG_MINIIMAGE_WIDTH*j+i] = palette[r];
      }
  backThumbTexture->UpdateBits(0, 0, MTG_MINIIMAGE_WIDTH, MTG_MINIIMAGE_HEIGHT, surface);
}

void CardEffect::UpdateBig(float dt)
{
  if (!GameOptions::GetInstance()->values[OPTIONS_PLASMAEFFECT].getIntValue()) return;
  static float t = 0;
  t += 3*dt;
  unsigned char c = (unsigned char)(9*t);
  for (int j = MTG_IMAGE_HEIGHT - 1; j >= 0; --j)
    for (int i = MTG_IMAGE_WIDTH - 1; i >= 0; --i)
      {
	unsigned int r = 2*c + sineTable2[0xFF & (i/2+j/2)] + sineTable1[0xFF & (j/2*(j/3+sineTable2[c])/256)];
	r %= 255;
	surface[MTG_IMAGE_WIDTH*j+i] = palette[r];
      }
  backTexture->UpdateBits(0, 0, MTG_IMAGE_WIDTH, MTG_IMAGE_HEIGHT, surface);
}
