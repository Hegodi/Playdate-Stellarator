#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "game.h"
#include "utils.h"
#include "stage.h"

SGame Game;
SStage Stage;

void LoadBitmaps()
{
	Game.mResources.mBall = loadImageAtPath(Game.mPd, "images/Ball");
	Game.mResources.mStageBackground = loadImageAtPath(Game.mPd, "images/StageBG");
	Game.mResources.mGun = loadImageAtPath(Game.mPd, "images/Gun");
	//BitmapTankHero = (LCDBitmap***)malloc(2 * sizeof(LCDBitmap**));
	//BitmapTankHero[0] = (LCDBitmap**)malloc(2 * sizeof(LCDBitmap*));
	//BitmapTankHero[0][0] = loadImageAtPath(PD, "images/TankHero_0");
	//BitmapTankHero[0][1] = loadImageAtPath(PD, "images/TankHero_1");
}

void InitGame(PlaydateAPI* pd) 
{
	Game.mPd = pd;
	LoadBitmaps();
	StageInit(&Stage);
}

void CleanupGame()
{

}

void UpdateInput()
{
}


int Update(void* ud)
{
	StageUpdate(&Stage);
	StageDraw(&Stage);
	Game.mPd->system->drawFPS(0, 0);
}

