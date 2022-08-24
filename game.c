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
	Game.mResources.mStageBackground = loadImageAtPath(Game.mPd, "images/StageBG");
	Game.mResources.mGun = loadImageAtPath(Game.mPd, "images/AtomsGun");
	Game.mResources.mGunFrame = (LCDBitmap**)malloc(2 * sizeof(LCDBitmap*));
	Game.mResources.mGunFrame[0] = loadImageAtPath(Game.mPd, "images/AtomsGun_Frame_a");
	Game.mResources.mGunFrame[1] = loadImageAtPath(Game.mPd, "images/AtomsGun_Frame_b");

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

	//Game.mResources.mFont = Game.mPd->graphics->fo

}

void InitGame(PlaydateAPI* pd) 
{
	Game.mPd = pd;
	LoadBitmaps();
	StageInit(&Stage);
}

void CleanupGame()
{
	Game.mPd->graphics->freeBitmap(Game.mResources.mStageBackground);
	Game.mPd->graphics->freeBitmap(Game.mResources.mGun);

	for (int i=0; i<4; i++)
	{
		Game.mPd->graphics->freeBitmap(Game.mResources.mAtom1[i]);
		Game.mPd->graphics->freeBitmap(Game.mResources.mAtom2[i]);
		Game.mPd->graphics->freeBitmap(Game.mResources.mAtom4[i]);
	}
	for (int i = 0; i < 2; i++)
	{
		Game.mPd->graphics->freeBitmap(Game.mResources.mAtomSelectedFX[i]);
		Game.mPd->graphics->freeBitmap(Game.mResources.mGunFrame[i]);
	}

	free(Game.mResources.mGunFrame);
	free(Game.mResources.mAtomSelectedFX);
	free(Game.mResources.mAtom1);
	free(Game.mResources.mAtom2);
	free(Game.mResources.mAtom4);
}

void UpdateInput()
{
}


int Update(void* ud)
{
	StageUpdate(&Stage);
	StageDraw(&Stage);
	//Game.mPd->system->drawFPS(0, 0);
}

