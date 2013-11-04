#ifndef _AI_H_
#define _AI_H_

#include "bullet.h"

#define AI_SPEED 25
class CAI{
	public:
		CAI(bool);
		CAI();
		~CAI();
		btVector3 makeMove(btVector3 puck, btVector3 paddle);
		bool isEnabled;
	
};

#endif
