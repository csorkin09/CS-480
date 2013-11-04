#include "gameManager.h"
#include <time.h>
#include <fstream>
#include <iostream>
using namespace std;

CGameManager::CGameManager(){
	isPaused = false;
	gameOver = false;
	p1Score = 0;
	p2Score = 0;
	timeSinceSound = 0;
}
CGameManager::~CGameManager(){
	
}

void CGameManager::reset(){
	bullet.puckRigidBody->translate(btVector3(-1 * bullet.puck.x + PUCK_X, -1 * bullet.puck.y + PUCK_Y, -1 * bullet.puck.z+ + PUCK_Z));
	bullet.paddle1RigidBody->translate(btVector3(-1 * bullet.paddle1.x +P1_X,-1 * bullet.paddle1.y +P1_Y,-1 * bullet.paddle1.z +P1_Z));
	bullet.paddle2RigidBody->translate(btVector3(-1 * bullet.paddle2.x +P2_X,-1 * bullet.paddle2.y +P2_Y,-1 * bullet.paddle2.z +P2_Z));
	
	bullet.puckRigidBody->clearForces();
	bullet.paddle1RigidBody->clearForces();
	bullet.paddle2RigidBody->clearForces();
	
	bullet.puckRigidBody->setLinearVelocity(btVector3(0,0,0));
	bullet.paddle1RigidBody->setLinearVelocity(btVector3(0,0,0));
	bullet.paddle2RigidBody->setLinearVelocity(btVector3(0,0,0));

}

// Update Game
void CGameManager::tick(float dt){
	timeSinceSound -= dt;
	
	// Update Bullet
	bullet.update(dt);
	
	// Check if game over
	if (bullet.puck.x < -4){
		p2Score++;
		reset();
		srand (time(NULL));
		bullet.serve(-1,0,rand()%10-5);	
	}
	if (bullet.puck.x > 4){
		p1Score++;
		reset();
		srand (time(NULL));
		bullet.serve(1,0,rand()%10-5);
	}
	
	if(p1Score >= 7 || p2Score >= 7){
		endGame();
	}
	else{	
		// Check if paddles are on their side
		if(bullet.paddle1.x > 0){
			bullet.paddle1RigidBody->translate(btVector3(-1 * bullet.paddle1.x-0.01, 0, 0));
			bullet.paddle1RigidBody->clearForces();
			bullet.paddle1RigidBody->setLinearVelocity(btVector3(0,0,0));
		}
	
		if(bullet.paddle2.x < 0){
			bullet.paddle2RigidBody->translate(btVector3(-1 * bullet.paddle2.x+0.01, 0, 0));
			bullet.paddle2RigidBody->clearForces();
			bullet.paddle2RigidBody->setLinearVelocity(btVector3(0,0,0));
		}
		
		// Make AI move
		bullet.paddle2.force(ai.makeMove(btVector3(bullet.puck.x,bullet.puck.y,bullet.puck.z),
			btVector3(bullet.paddle2.x,bullet.paddle2.y,bullet.paddle2.z)));
		
		// Check Collisions
		if(bullet.collisionDetected && timeSinceSound <= 0){	
			audio.playSFX(1);
			timeSinceSound = 1.0;
		}
	}
}

void CGameManager::endGame(){
	char scoreStr[4];
	int scores[2][10];
	
	for(int i=0; i<2; i++){
		for(int j=0; j<10; j++){
			scores[i][j] = 0;	
		}
	}
	
	gameOver=true;
	
	ifstream fin;
	fin.open("highscores.txt");
	
	int index = 0;
	bool flag = false;
	int winIndex = -1;
	fin.read(scoreStr,3);
	while(!fin.eof()){
		int p1 = scoreStr[0] - '0';
		int p2 = scoreStr[1] - '0';
		
		cout << p1 << ' ' << p2 << endl;
		
		if(p1-p2 >= p1Score - p2Score || flag){
			sprintf(gameOverStrings[index], "%i-%i", p1, p2);
			scores[0][index] = p1;
			scores[1][index] = p2;
			index++;			
		}
		else if(!flag){
			cout << p1Score << ' ' << p2Score << " <--- Your Score!" << endl;
			sprintf(gameOverStrings[index], "%i-%i <-- Your Score!", p1Score, p2Score);
		
			scores[0][index] = p1Score;
			scores[1][index] = p2Score;
			winIndex = index;			
			index++;
			
			sprintf(gameOverStrings[index], "%i-%i", p1, p2);
			scores[0][index] = p1;
			scores[1][index] = p2;
			index++;		
			flag = true;
		}
		fin.read(scoreStr,3);
	}
	
	fin.close();
	
	ofstream fout;
	fout.open("highscores.txt");
	
	for(int j=0; j<10; j++){
		fout << scores[0][j] << scores[1][j] << endl;

	}	
}
