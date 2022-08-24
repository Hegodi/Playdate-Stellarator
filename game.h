#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include "pd_api.h"
#include "stage.h"

#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 240

typedef struct
{
	PlaydateAPI* mPd;
	struct
	{
		LCDBitmap* mStageBackground;
		LCDBitmap* mGun;
		LCDBitmap** mGunFrame;
		LCDBitmap** mAtom1;
		LCDBitmap** mAtom2;
		LCDBitmap** mAtom4;
		LCDBitmap** mAtomSelectedFX;
		LCDFont* mFont;
	} mResources;
} SGame;


// Game data
extern SGame Game;

int Update(void* ud);
void InitGame(PlaydateAPI* pd);
void CleanupGame();

#endif 
