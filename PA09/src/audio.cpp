#include "audio.h"
#include <iostream>

using namespace std;
CAudio::CAudio(){
	SDL_Init( SDL_INIT_EVERYTHING );
	
	//Initialize SDL_mixer
	if( Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 4096 ) != -1 )
	{
		Mix_Music *music = NULL;
		
		//Load the music 
		music = Mix_LoadMUS( "../assets/terran.mp3" ); 
		Mix_PlayMusic( music, -1 );
		collisionSFX = Mix_LoadWAV( "../assets/laser.wav" );
	}
}


CAudio::~CAudio(){
	SDL_Quit();	
}

void CAudio::playSFX(int index){
	Mix_PlayChannel( -1, collisionSFX, 0 );
	//Mix_PlayMusic( collisionSFX, -1 );
}
