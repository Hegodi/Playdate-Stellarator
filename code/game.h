#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include "pd_api.h"
#include "stage.h"

#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 240

#define MAX_SCORES 5

typedef enum {EMode_SplashScreen, EMode_Menu, EMode_InGame, EMode_Scoreboard, EMode_Tutorial} EMode;
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

	int mScoreboard[3*MAX_SCORES];

	struct
	{
		LCDBitmap* mMenuBackground;
		LCDBitmap* mMenuTitle;
		LCDBitmap* mMenuLogo;
		LCDBitmap* mMenuFrame;
		LCDBitmap* mMarkerMenu;

		LCDBitmap* mStageBackground;
		LCDBitmap** mStageDamage;
		LCDBitmap** mBalls;
		LCDBitmap** mAtomSelectedFX;
		LCDBitmap** mGrabberFX;
		LCDBitmap** mExplosionFX;
		LCDBitmap* mSocket;
		LCDBitmap* mArrow;

		LCDFont* mFont;

	} mResources;

	SamplePlayer* mSamplePlayer;

} SGame;


// Game data
extern SGame Game;

int Update(void* ud);
void InitGame(PlaydateAPI* pd);
void CleanupGame();
void AddScore(int score, int level);

#endif 
