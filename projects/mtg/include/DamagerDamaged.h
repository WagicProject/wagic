#ifndef _DAMAGERDAMAGED_H_
#define _DAMAGERDAMAGED_H_

#include "../include/CardGui.h"

class Player;

class DamagerDamaged:public CardGui{
public:
	Player * damageSelecter;
	int mCount;
	Damage * damages[10];
	int damageToDeal;

	int dealOneDamage(DamagerDamaged * target);
	int addDamage(int damage, DamagerDamaged * source);
	int removeDamagesTo(DamagerDamaged * target);
	int removeDamagesFrom(DamagerDamaged * source);
	int sumDamages();
	int hasLethalDamage();
	DamagerDamaged(CardGui * cardg, Player * _damageSelecter, bool _hasFocus); 
	~DamagerDamaged();
	void Render(Player * currentPlayer);



};


#endif