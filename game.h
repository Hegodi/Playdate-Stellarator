#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include "pd_api.h"
#include "stage.h"

#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 240

typedef enum {EMode_SplashScreen, EMode_Menu, EMode_InGame} EMode;
typedef struct
{
	PlaydateAPI* mPd;
	EMode mMode;

	struct
	{
		int mIndexSel;
	} mMenu;

	struct
	{
		float mX; 
		float mY;
		int mTicks;
		int mFlag;
		SBall* balls;
	} mSplash;

	struct
	{
		LCDBitmap* mMenuBackground;
		LCDBitmap* mSplashBackground;
		LCDBitmap* mSplashTitle;
		LCDBitmap* mStageBackground;
		LCDBitmap** mStageDamage;
		LCDBitmap* mMarkerMenu;
		LCDBitmap** mBalls;
		LCDBitmap** mAtomSelectedFX;
		LCDBitmap** mExplosionFX;
		LCDBitmap* mSocket;

		LCDFont* mFont;
	} mResources;
} SGame;


// Game data
extern SGame Game;

int Update(void* ud);
void InitGame(PlaydateAPI* pd);
void CleanupGame();

#endif 
