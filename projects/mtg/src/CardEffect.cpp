#include "../include/GameApp.h"
#include "../include/MTGCard.h"
#include "../include/GameOptions.h"
#include "../include/CardEffect.h"

CardEffect::CardEffect(CardGui* target) :
    target(target)
{

}

CardEffect::~CardEffect()
{

}

void CardEffect::Render()
{
    //  std::cout << "Rendering effect" << std::endl;
}
