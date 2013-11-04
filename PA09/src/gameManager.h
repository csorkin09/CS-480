#include "bullet.h"
#include "ai.h"
#include "audio.h"

class CGameManager{
	public:
		bool isPaused;
		bool gameOver;
		int p1Score;
		int p2Score;
		float timeSinceSound;
		char gameOverStrings[10][20];
		CBullet bullet;
		CAI ai;
		CAudio audio;
		
		CGameManager();
		~CGameManager();	
		void reset();
		void tick(float);
		void endGame();
};
