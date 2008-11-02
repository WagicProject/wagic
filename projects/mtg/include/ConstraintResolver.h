#ifndef _CONSTRAINTRESOLVER_H_
#define _CONSTRAINTRESOLVER_H_

#include "GameObserver.h"
#include "MTGCardInstance.h"


class ConstraintResolver {
protected:
public:
	static int untap(GameObserver * game, MTGCardInstance * card);
};

#endif
