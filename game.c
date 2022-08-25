#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "game.h"
#include "utils.h"
#include "stage.h"


#define FILE_SCOREBOARD "scoreboard"

SGame Game;
SStage Stage;

static SStageConfig StageConfigs[] = { 
	{ 0, 4, 10, 60, 300 } ,
	{ 1, 3, 10, 60, 240 } ,
	{ 2, 1, 10, 60, 180 } 
};

static const char* MenuStrings[] = {
	"Tutorial",
	"Easy",
	"Normal",
	"Hard",
	"Scoreboard"
};

void LoadScoreboard()
{
	SDFile* file = Game.mPd->file->open(FILE_SCOREBOARD, kFileReadData);
	if (file != NULL)
	{
		Game.mPd->file->read(file, &Game.mScoreboard, sizeof(Game.mScoreboard));
		Game.mPd->file->close(file);
	}
	else
	{
		memset(&Game.mScoreboard, sizeof(Game.mScoreboard), 0);
	}
}

void SaveScoreboard()
{
	SDFile* file = Game.mPd->file->open(FILE_SCOREBOARD, kFileWrite);
	if (file != NULL)
	{
		Game.mPd->file->write(file, &Game.mScoreboard, sizeof(Game.mScoreboard));
	}
	Game.mPd->file->close(file);

}

void AddScore(int score, int level)
{
	int ind = level * MAX_SCORES;
	for (int i=0; i<MAX_SCORES; i++, ind++)
	{
		if (score > Game.mScoreboard[ind])
		{
			Game.mScoreboard[ind] = score;
			break;
		}
	}
	SaveScoreboard();
}

void UpdateMenu()
{
	Game.mPd->graphics->clear(1);
	Game.mPd->graphics->drawBitmap(Game.mResources.mMenuBackground, 0, 0, kBitmapUnflipped);
	Game.mPd->graphics->setFont(Game.mResources.mFont);
	int x = SCREEN_WIDTH / 2;
	Game.mPd->graphics->fillRect(SCREEN_WIDTH / 2-60, 90, 120, 120, kColorWhite);
	Game.mPd->graphics->drawRect(SCREEN_WIDTH / 2-60, 90, 120, 120, kColorBlack);
	for (int i=0; i<5; i++)
	{
		int y = 100 + i * 20;
		DrawText(Game.mPd, MenuStrings[i], x, y , Game.mResources.mFont, true);
		if (i == Game.mMenu.mIndexSel)
		{
			Game.mPd->graphics->drawBitmap(Game.mResources.mMarkerMenu, x-16 - 50, y, kBitmapUnflipped);
			Game.mPd->graphics->drawBitmap(Game.mResources.mMarkerMenu, x+50, y, kBitmapFlippedX);
		}
	}

	PDButtons current;
	PDButtons pushed;
	Game.mPd->system->getButtonState(&current, &pushed, NULL);
	if (pushed & kButtonUp)
	{
		if (Game.mMenu.mIndexSel > 0) Game.mMenu.mIndexSel--;

	}
	else if (pushed & kButtonDown)
	{
		if (Game.mMenu.mIndexSel < 4) Game.mMenu.mIndexSel++;
	}
	else if (pushed & kButtonA)
	{
		if (Game.mMenu.mIndexSel > 0 && Game.mMenu.mIndexSel < 4)
		{
			StageInit(&Stage, &StageConfigs[Game.mMenu.mIndexSel-1]);
			Game.mMode = EMode_InGame;
		}
	}
}


void UpdateSplashScreen()
{
	Game.mSplash.mTicks++;
	Game.mPd->graphics->clear(1);
	Game.mPd->graphics->drawBitmap(Game.mResources.mSplashBackground, 0, 0, kBitmapUnflipped);

	if (Game.mSplash.mFlag == 0)
	{
		if (Game.mSplash.mTicks > 30)
		{
			Game.mSplash.mFlag = 1;
			Game.mSplash.mTicks = 0;
		}
		DrawText(Game.mPd, "Hegodi Games", SCREEN_WIDTH / 2, 200, Game.mResources.mFont, true);
	}
	if (Game.mSplash.mFlag == 1)
	{
		DrawText(Game.mPd, "Hegodi Games", SCREEN_WIDTH / 2, 200, Game.mResources.mFont, true);
		float factor = 1.0f / (1.0f + Game.mSplash.mTicks * 0.05f);
		if (abs(Game.mSplash.mX) > 5.0f)
		{
			Game.mSplash.mX += 10.0f * factor;
		}
		Game.mSplash.mY = -abs(cos(Game.mSplash.mTicks * 0.1f) * 500) * factor * factor; 

		if (Game.mSplash.mTicks > 180)
		{
			Game.mSplash.mFlag = 2;
			Game.mSplash.mTicks = 0;
			Game.mSplash.mX = 0;
			Game.mSplash.mY = 0;
		}
		Game.mPd->graphics->drawBitmap(Game.mResources.mSplashTitle, Game.mSplash.mX, Game.mSplash.mY, kBitmapUnflipped);
	}
	else if (Game.mSplash.mFlag == 2)
	{
		DrawText(Game.mPd, "Hegodi Games", SCREEN_WIDTH / 2, 200 + Game.mSplash.mY, Game.mResources.mFont, true);
		Game.mPd->graphics->drawBitmap(Game.mResources.mSplashTitle, Game.mSplash.mX, Game.mSplash.mY, kBitmapUnflipped);
		Game.mPd->graphics->drawBitmap(Game.mResources.mMenuBackground, Game.mSplash.mX, Game.mSplash.mY + SCREEN_HEIGHT, kBitmapUnflipped);
		Game.mSplash.mY -= 5.0f;
		if (Game.mSplash.mY < -SCREEN_HEIGHT)
		{
			Game.mMode = EMode_Menu;
		}
	}
}

void UpdateScoreboard()
{

}

void LoadBitmaps()
{
	Game.mResources.mStageBackground = loadImageAtPath(Game.mPd, "images/StageBG");

	Game.mResources.mBalls = (LCDBitmap**)malloc(6 * sizeof(LCDBitmap*));
	Game.mResources.mBalls[0] = loadImageAtPath(Game.mPd, "images/Ball2");
	Game.mResources.mBalls[1] = loadImageAtPath(Game.mPd, "images/Ball4");
	Game.mResources.mBalls[2] = loadImageAtPath(Game.mPd, "images/Ball8");
	Game.mResources.mBalls[3] = loadImageAtPath(Game.mPd, "images/Ball16");
	Game.mResources.mBalls[4] = loadImageAtPath(Game.mPd, "images/Ball32");
	Game.mResources.mBalls[5] = loadImageAtPath(Game.mPd, "images/Ball64");

	Game.mResources.mAtomSelectedFX = (LCDBitmap**)malloc(3 * sizeof(LCDBitmap*));
	Game.mResources.mAtomSelectedFX[0] = loadImageAtPath(Game.mPd, "images/AtomHLa");
	Game.mResources.mAtomSelectedFX[1] = loadImageAtPath(Game.mPd, "images/AtomHLb");
	Game.mResources.mAtomSelectedFX[2] = loadImageAtPath(Game.mPd, "images/AtomHLc");

	Game.mResources.mGrabberFX = (LCDBitmap**)malloc(3 * sizeof(LCDBitmap*));
	Game.mResources.mGrabberFX[0] = loadImageAtPath(Game.mPd, "images/Ball_Selected0");
	Game.mResources.mGrabberFX[1] = loadImageAtPath(Game.mPd, "images/Ball_Selected1");
	Game.mResources.mGrabberFX[2] = loadImageAtPath(Game.mPd, "images/Ball_Selected2");

	Game.mResources.mExplosionFX = (LCDBitmap**)malloc(5 * sizeof(LCDBitmap*));
	Game.mResources.mExplosionFX[0] = loadImageAtPath(Game.mPd, "images/Explosion1");
	Game.mResources.mExplosionFX[1] = loadImageAtPath(Game.mPd, "images/Explosion2");
	Game.mResources.mExplosionFX[2] = loadImageAtPath(Game.mPd, "images/Explosion3");
	Game.mResources.mExplosionFX[3] = loadImageAtPath(Game.mPd, "images/Explosion4");
	Game.mResources.mExplosionFX[4] = loadImageAtPath(Game.mPd, "images/Explosion5");

	Game.mResources.mSocket = loadImageAtPath(Game.mPd, "images/Socket");

	Game.mResources.mMenuBackground = loadImageAtPath(Game.mPd, "images/MenuBG");
	Game.mResources.mSplashBackground = loadImageAtPath(Game.mPd, "images/GrayBG");
	Game.mResources.mSplashTitle = loadImageAtPath(Game.mPd, "images/Title");
	Game.mResources.mMarkerMenu = loadImageAtPath(Game.mPd, "images/MarkerMenu");

	Game.mResources.mStageDamage = (LCDBitmap**)malloc(4 * sizeof(LCDBitmap*));
	Game.mResources.mStageDamage[0] = loadImageAtPath(Game.mPd, "images/StageOveraly_Damage3");
	Game.mResources.mStageDamage[1] = loadImageAtPath(Game.mPd, "images/StageOveraly_Damage2");
	Game.mResources.mStageDamage[2] = loadImageAtPath(Game.mPd, "images/StageOveraly_Damage1");
	Game.mResources.mStageDamage[3] = loadImageAtPath(Game.mPd, "images/StageOveraly_Damage0");

	Game.mResources.mArrow = loadImageAtPath(Game.mPd, "images/Arrow");

	const char *outErr;
	Game.mResources.mFont = Game.mPd->graphics->loadFont("fonts/font-full-circle", &outErr);

}

void InitGame(PlaydateAPI* pd) 
{
	Game.mPd = pd;
	LoadBitmaps();
	Game.mMode = EMode_SplashScreen;

	Game.mSplash.mX = -SCREEN_WIDTH;
	Game.mSplash.mY = 0;
	Game.mSplash.mFlag = 0;
	Game.mSplash.mTicks = 0;

	Game.mMenu.mIndexSel = 0;
	LoadScoreboard();
}

void CleanupGame()
{
	Game.mPd->graphics->freeBitmap(Game.mResources.mStageBackground);
	Game.mPd->graphics->freeBitmap(Game.mResources.mSocket);
	Game.mPd->graphics->freeBitmap(Game.mResources.mMenuBackground);
	Game.mPd->graphics->freeBitmap(Game.mResources.mSplashBackground);
	Game.mPd->graphics->freeBitmap(Game.mResources.mSplashTitle);
	Game.mPd->graphics->freeBitmap(Game.mResources.mArrow);

	for (int i=0; i<6; i++)
	{
		Game.mPd->graphics->freeBitmap(Game.mResources.mBalls[i]);
	}
	for (int i = 0; i < 3; i++)
	{
		Game.mPd->graphics->freeBitmap(Game.mResources.mGrabberFX[i]);
		Game.mPd->graphics->freeBitmap(Game.mResources.mAtomSelectedFX[i]);
	}
	for (int i = 0; i < 4; i++)
	{
		Game.mPd->graphics->freeBitmap(Game.mResources.mStageDamage[i]);
	}

	free(Game.mResources.mGrabberFX);
	free(Game.mResources.mStageDamage);
	free(Game.mResources.mAtomSelectedFX);
	free(Game.mResources.mBalls);
}

int Update(void* ud)
{
	switch (Game.mMode)
	{
	case EMode_SplashScreen:
		UpdateSplashScreen();
		break;
	case EMode_Menu:
		UpdateMenu();
		break;
	case EMode_InGame:
		StageUpdate(&Stage);
		break;
	default:
		break;
	}
	//Game.mPd->system->drawFPS(0, 0);
}


