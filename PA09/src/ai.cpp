#include "ai.h"
#include <stdio.h>

CAI::CAI(bool enable){
	isEnabled = enable;
}

CAI::CAI(){
	isEnabled = true;
}

CAI::~CAI(){

}

btVector3 CAI::makeMove(btVector3 puck, btVector3 paddle){
	btVector3 returnVec(0.0,0.0,0.0);
	if(isEnabled){
		if (paddle.getZ() < puck.getZ()){
			returnVec.setZ(AI_SPEED);
		}
		else{
			returnVec.setZ(-AI_SPEED);
		}
		
		if(puck.getX() > 0){ // puck on our side
			if(paddle.getX() < puck.getX()){
				returnVec.setX(AI_SPEED);
			}
			else{
				returnVec.setX(-AI_SPEED);
			}
		}
		else if(paddle.getX() > 2){
			returnVec.setX(-AI_SPEED);	
		}
	}
	return returnVec;
}
