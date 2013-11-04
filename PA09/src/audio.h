#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"
#include "SDL/SDL_mixer.h"

class CAudio{
	public:
		Mix_Chunk *collisionSFX;
		
		CAudio();
		~CAudio();
		void playSFX(int);
};
