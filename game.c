#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "game.h"
#include "utils.h"
#include "stage.h"

SGame Game;
SStage Stage;

static SStageConfig StageConfigs[] = { 
	{ 3, 16, 60, 300 } ,
	{ 3, 16, 60, 240 } ,
	{ 3, 16, 30, 160 } 
};


void LoadBitmaps()
{
	Game.mResources.mStageBackground = loadImageAtPath(Game.mPd, "images/StageBG");

	Game.mResources.mAtom1 = (LCDBitmap**)malloc(4 * sizeof(LCDBitmap*));
	Game.mResources.mAtom1[0] = loadImageAtPath(Game.mPd, "images/Atom1a");
	Game.mResources.mAtom1[1] = loadImageAtPath(Game.mPd, "images/Atom1b");
	Game.mResources.mAtom1[2] = loadImageAtPath(Game.mPd, "images/Atom1c");
	Game.mResources.mAtom1[3] = loadImageAtPath(Game.mPd, "images/Atom1d");

	Game.mResources.mAtom2 = (LCDBitmap**)malloc(4 * sizeof(LCDBitmap*));
	Game.mResources.mAtom2[0] = loadImageAtPath(Game.mPd, "images/Atom2a");
	Game.mResources.mAtom2[1] = loadImageAtPath(Game.mPd, "images/Atom2b");
	Game.mResources.mAtom2[2] = loadImageAtPath(Game.mPd, "images/Atom2c");
	Game.mResources.mAtom2[3] = loadImageAtPath(Game.mPd, "images/Atom2d");

	Game.mResources.mAtom4 = (LCDBitmap**)malloc(4 * sizeof(LCDBitmap*));
	Game.mResources.mAtom4[0] = loadImageAtPath(Game.mPd, "images/Atom4a");
	Game.mResources.mAtom4[1] = loadImageAtPath(Game.mPd, "images/Atom4b");
	Game.mResources.mAtom4[2] = loadImageAtPath(Game.mPd, "images/Atom4c");
	Game.mResources.mAtom4[3] = loadImageAtPath(Game.mPd, "images/Atom4d");

	Game.mResources.mAtomSelectedFX = (LCDBitmap**)malloc(2 * sizeof(LCDBitmap*));
	Game.mResources.mAtomSelectedFX[0] = loadImageAtPath(Game.mPd, "images/AtomHLa");
	Game.mResources.mAtomSelectedFX[1] = loadImageAtPath(Game.mPd, "images/AtomHLb");

	const char *outErr;
	Game.mResources.mFont = Game.mPd->graphics->loadFont("fonts/font-full-circle", &outErr);

}

void InitGame(PlaydateAPI* pd) 
{
	Game.mPd = pd;
	LoadBitmaps();

	StageInit(&Stage, &StageConfigs[0]);
}

void CleanupGame()
{
	Game.mPd->graphics->freeBitmap(Game.mResources.mStageBackground);

	for (int i=0; i<4; i++)
	{
		Game.mPd->graphics->freeBitmap(Game.mResources.mAtom1[i]);
		Game.mPd->graphics->freeBitmap(Game.mResources.mAtom2[i]);
		Game.mPd->graphics->freeBitmap(Game.mResources.mAtom4[i]);
	}
	for (int i = 0; i < 2; i++)
	{
		Game.mPd->graphics->freeBitmap(Game.mResources.mAtomSelectedFX[i]);
	}

	free(Game.mResources.mAtomSelectedFX);
	free(Game.mResources.mAtom1);
	free(Game.mResources.mAtom2);
	free(Game.mResources.mAtom4);
}

int Update(void* ud)
{
	StageUpdate(&Stage);
	StageDraw(&Stage);
	//Game.mPd->system->drawFPS(0, 0);
}


