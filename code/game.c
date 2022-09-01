#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "game.h"
#include "utils.h"
#include "stage.h"


#define FILE_SCOREBOARD "scoreboard"

SGame Game;
SStage Stage;

//========================================================================================================
// Static data 

static SStageConfig StageConfigs[] = { 
	{ 0, 3, 13, 0, 0, 0} ,
	{ 1, 3, 13, 3, 180, 90} ,
	{ 2, 3, 13, 4, 120, 60} ,
	{ 3, 3, 13, 5, 90, 30} 
};

static const char* MenuStrings[] = {
	"Tutorial",
	"Easy",
	"Normal",
	"Hard",
	"Scoreboard"
};

#define NOTE_C 262
#define NOTE_D 294
#define NOTE_E 330
#define NOTE_F 349
#define NOTE_G 392
#define NOTE_A 440
#define NOTE_B 494

static AudioNoteData AudioDataMenuSelect[] = { {EAudioShape_Sin , NOTE_G, 0.2f, 0.2f, 0.2f, 0.2f },
											 };

static AudioNoteData AudioDataMenuClick[] = { {EAudioShape_Sin , NOTE_F, 0.05f, 0.2f, 0.f, 0.2f },
											  {EAudioShape_Sin , 2 * NOTE_G, 0.10f, 0.0f, 0.5f, 0.3f},
											};

static AudioNoteData AudioDataInGameGrabEmpty[] = { {EAudioShape_NoiseSquare , 50, 1.5f, 1.0, 0.0f, 0.10f } };

static AudioNoteData AudioDataInGameGrabFull[] = { {EAudioShape_NoiseSquare , 100, 1.5f, 0.1, 0.9f, 0.15f } };

static AudioNoteData AudioDataInGameShoot[] = { {EAudioShape_NoiseSquare , 200, 0.1f, 0.0, 0.1f, 0.2f } ,
												{EAudioShape_NoiseSquare , 100, 1.0f, 0.0f, 0.5f, 0.1f},
											  };

static AudioNoteData AudioDataMergeFail[]   = { {EAudioShape_Sin , 200, 0.05f, 0.1, 0.0f, 0.1f } ,
												{EAudioShape_Sin , 500, 0.2f, 0.0f, 1.0f, 0.2f},
											  };

static AudioNoteData AudioDataMergeDone[]   = { {EAudioShape_Sin , 800, 0.2f, 0.1f, 0.1f, 0.3f },
                                                {EAudioShape_Sin , 400, 0.4f, 0.1f, 0.5f, 0.15f },
											  };

static AudioNoteData AudioTest2[]          =  {{EAudioShape_Square , NOTE_C, 0.3f, 0.5f, 0.0f, 0.1f },
                                               {EAudioShape_Square , NOTE_D, 0.2f, 0.0f, 0.2f, 0.1f },
                                               {EAudioShape_Square , NOTE_E, 0.2f, 0.0f, 0.2f, 0.1f },
                                               {EAudioShape_Square , NOTE_F, 0.2f, 0.0f, 0.2f, 0.1f },
                                               {EAudioShape_Square , NOTE_A, 0.2f, 0.0f, 0.2f, 0.1f },
                                               {EAudioShape_Square , NOTE_B, 0.2f, 0.0f, 0.2f, 0.1f },
											 };

static AudioNoteData AudioIntroA[] = { {EAudioShape_Sin , NOTE_E, 0.1f, 0.1f, 0.1f, 0.1f },
									   {EAudioShape_Sin , NOTE_A, 0.2f, 0.1f, 0.1f, 0.1f },
									   {EAudioShape_Sin , NOTE_C, 0.1f, 0.1f, 0.1f, 0.1f },
									   {EAudioShape_Sin , NOTE_B, 0.3f, 0.1f, 0.1f, 0.1f },
									   {EAudioShape_Sin , NOTE_A, 0.5f, 0.1f, 0.8f, 0.1f },
									  };


static AudioNoteData AudioIntroB[] = {  {EAudioShape_Noise, 200, 0.8f, 0.1f, 0.1f, 0.1f } ,
								        {EAudioShape_Sin , NOTE_A, 0.2f, 0.1f, 0.1f, 0.1f },
										{EAudioShape_Sin , NOTE_C, 0.1f, 0.1f, 0.1f, 0.1f },
										{EAudioShape_Sin , NOTE_B, 0.3f, 0.1f, 0.1f, 0.1f },
										{EAudioShape_Sin , NOTE_A, 0.1f, 0.1f, 0.1f, 0.1f },
										{EAudioShape_Sin , NOTE_A, 0.2f, 0.1f, 0.1f, 0.1f },
										{EAudioShape_Sin , NOTE_G, 0.2f, 0.1f, 0.1f, 0.1f },
										{EAudioShape_Sin , NOTE_F, 0.8f, 0.1f, 0.5f, 0.1f },
										{EAudioShape_Sin , NOTE_A, 0.2f, 0.1f, 0.1f, 0.1f },
										{EAudioShape_Sin , NOTE_C, 0.1f, 0.1f, 0.1f, 0.1f },
										{EAudioShape_Sin , NOTE_B, 0.3f, 0.1f, 0.1f, 0.1f },
										{EAudioShape_Sin , NOTE_A, 0.1f, 0.1f, 0.1f, 0.1f },
										{EAudioShape_Sin , NOTE_A, 0.2f, 0.1f, 0.1f, 0.1f },
										{EAudioShape_Sin , NOTE_G, 0.2f, 0.1f, 0.1f, 0.1f },
										{EAudioShape_Sin , NOTE_F, 0.8f, 0.1f, 0.5f, 0.1f },
										{EAudioShape_Sin , NOTE_A, 0.1f, 0.1f, 0.1f, 0.1f },
										{EAudioShape_Sin , NOTE_A, 0.2f, 0.1f, 0.1f, 0.1f },
										{EAudioShape_Sin , NOTE_G, 0.2f, 0.1f, 0.1f, 0.1f },
										{EAudioShape_Sin , NOTE_F, 0.8f, 0.1f, 0.5f, 0.1f },
};
static AudioNoteData AudioIntroC[] = { {EAudioShape_Noise, 200, 3.0f, 0.2f, 0.2f, 0.05f } };


static AudioNoteData AudioIntro[]         =  { {EAudioShape_Sin , NOTE_A, 0.2f, 0.1f, 0.1f, 0.1f },
                                               {EAudioShape_Sin , NOTE_C, 0.1f, 0.1f, 0.1f, 0.1f },
                                               {EAudioShape_Sin , NOTE_B, 0.3f, 0.1f, 0.1f, 0.1f },
                                               {EAudioShape_Sin , NOTE_A, 0.5f, 0.1f, 0.8f, 0.1f },
                                               {EAudioShape_NoiseSquare, 2000, 1.0f, 1.0f, 0.0f, 0.1f },
                                               {EAudioShape_Sin , NOTE_A, 0.2f, 0.1f, 0.1f, 0.1f },
                                               {EAudioShape_Sin , NOTE_C, 0.1f, 0.1f, 0.1f, 0.1f },
                                               {EAudioShape_Sin , NOTE_B, 0.3f, 0.1f, 0.1f, 0.1f },
                                               {EAudioShape_Sin , NOTE_A, 0.1f, 0.1f, 0.1f, 0.1f },
                                               {EAudioShape_Sin , NOTE_A, 0.2f, 0.1f, 0.1f, 0.1f },
                                               {EAudioShape_Sin , NOTE_G, 0.2f, 0.1f, 0.1f, 0.1f },
                                               {EAudioShape_Sin , NOTE_F, 0.8f, 0.1f, 0.5f, 0.1f },
                                               {EAudioShape_Sin , NOTE_A, 0.2f, 0.1f, 0.1f, 0.1f },
                                               {EAudioShape_Sin , NOTE_C, 0.1f, 0.1f, 0.1f, 0.1f },
                                               {EAudioShape_Sin , NOTE_B, 0.3f, 0.1f, 0.1f, 0.1f },
                                               {EAudioShape_Sin , NOTE_A, 0.1f, 0.1f, 0.1f, 0.1f },
                                               {EAudioShape_Sin , NOTE_A, 0.2f, 0.1f, 0.1f, 0.1f },
                                               {EAudioShape_Sin , NOTE_G, 0.2f, 0.1f, 0.1f, 0.1f },
                                               {EAudioShape_Sin , NOTE_F, 0.8f, 0.1f, 0.5f, 0.1f },
                                               {EAudioShape_Sin , NOTE_A, 0.1f, 0.1f, 0.1f, 0.1f },
                                               {EAudioShape_Sin , NOTE_A, 0.2f, 0.1f, 0.1f, 0.1f },
                                               {EAudioShape_Sin , NOTE_G, 0.2f, 0.1f, 0.1f, 0.1f },
                                               {EAudioShape_Sin , NOTE_F, 0.8f, 0.1f, 0.5f, 0.1f },
                                               {EAudioShape_NoiseSquare, 500, 2.0f, 0.1f, 0.0f, 0.1f },
                                               {EAudioShape_Sin , NOTE_A, 0.1f, 0.1f, 0.1f, 0.1f },
                                               {EAudioShape_Sin , NOTE_G, 0.1f, 0.1f, 0.1f, 0.1f },
                                               {EAudioShape_Sin , NOTE_F, 0.4f, 0.1f, 0.5f, 0.1f },
											 };



//========================================================================================================

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

void DrawGeneralBackground(int x, int y, bool title)
{
	Game.mPd->graphics->drawBitmap(Game.mResources.mMenuBackground, x, y, kBitmapUnflipped);
	Game.mPd->graphics->drawBitmap(Game.mResources.mMenuFrame, x, y, kBitmapUnflipped);
	Game.mPd->graphics->drawBitmap(Game.mResources.mMenuLogo, SCREEN_WIDTH/2 - 64 + 120, 75, kBitmapUnflipped);
	Game.mPd->graphics->drawBitmap(Game.mResources.mMenuLogo, SCREEN_WIDTH/2 - 64 - 120, 75, kBitmapUnflipped);
	if (title)
	{
		Game.mPd->graphics->drawBitmap(Game.mResources.mMenuTitle, SCREEN_WIDTH / 2 - 128 + x, 16 + y, kBitmapUnflipped);
	}
}

void UpdateMenu()
{
	DrawGeneralBackground(0, 0, true);

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
		if (Game.mMenu.mIndexSel > 0)
		{
			Game.mMenu.mIndexSel--;
			Game.mPd->sound->sampleplayer->setSample(Game.mSamplePlayer, Game.mResources.mAudio.mSampleMenuSelect);
			Game.mPd->sound->sampleplayer->play(Game.mSamplePlayer, 1, 1.0f);
		}

	}
	else if (pushed & kButtonDown)
	{
		if (Game.mMenu.mIndexSel < 4)
		{
			Game.mMenu.mIndexSel++;
			Game.mPd->sound->sampleplayer->setSample(Game.mSamplePlayer, Game.mResources.mAudio.mSampleMenuSelect);
			Game.mPd->sound->sampleplayer->play(Game.mSamplePlayer, 1, 1.0f);
		}

	}
	else if (pushed & kButtonA)
	{
		Game.mPd->sound->sampleplayer->setSample(Game.mSamplePlayer, Game.mResources.mAudio.mSampleMenuClick);
		Game.mPd->sound->sampleplayer->play(Game.mSamplePlayer, 1, 1.0f);
		if (Game.mMenu.mIndexSel > 0 && Game.mMenu.mIndexSel < 4)
		{
			StageInit(&Stage, &StageConfigs[Game.mMenu.mIndexSel]);
			Game.mMode = EMode_InGame;
		}
		else if (Game.mMenu.mIndexSel == 4)
		{
			Game.mMode = EMode_Scoreboard;
		}
		else if (Game.mMenu.mIndexSel == 0)
		{
			StageInit(&Stage, &StageConfigs[Game.mMenu.mIndexSel]);
			Game.mMode = EMode_Tutorial;
		}
	}
}

void UpdateSplashScreen()
{
	Game.mSplash.mTicks++;
	Game.mPd->graphics->drawBitmap(Game.mResources.mMenuBackground, 0, 0, kBitmapUnflipped);

	if (Game.mSplash.mFlag == 0)
	{
		if (Game.mSplash.mTicks > 90)
		{
			Game.mSplash.mFlag = 1;
			Game.mSplash.mTicks = 0;
			Game.mSplash.mY = -200;
			Game.mSplash.mX = -200;
			Game.mPd->sound->sampleplayer->setSample(Game.mSamplePlayer, Game.mResources.mAudio.mSampleIntroB);
			Game.mPd->sound->sampleplayer->play(Game.mSamplePlayer, 1, 1.0f);

		}
		DrawText(Game.mPd, "Hegodi Games", SCREEN_WIDTH / 2, 200, Game.mResources.mFont, true);
	}
	if (Game.mSplash.mFlag == 1)
	{
		DrawText(Game.mPd, "Hegodi Games", SCREEN_WIDTH / 2, 200, Game.mResources.mFont, true);
		Game.mPd->graphics->drawBitmap(Game.mResources.mMenuTitle, SCREEN_WIDTH/2 - 128, Game.mSplash.mY + 10, kBitmapUnflipped);
		Game.mPd->graphics->drawBitmap(Game.mResources.mMenuLogo, SCREEN_WIDTH/2 - 64 + 120, Game.mSplash.mX + 75, kBitmapUnflipped);
		Game.mPd->graphics->drawBitmap(Game.mResources.mMenuLogo, SCREEN_WIDTH/2 - 64 - 120, Game.mSplash.mX + 75, kBitmapUnflipped);

		float factor = 5.0 / (1.0 + Game.mSplash.mTicks);
		float factorSqr = factor * factor;
		Game.mSplash.mY = -200 * cosf(Game.mSplash.mTicks/2.0f) * factorSqr;
		Game.mSplash.mX = -200 * cosf(Game.mSplash.mTicks/4.0f) * factorSqr;

		if (Game.mSplash.mTicks > 180)
		{
			Game.mSplash.mFlag = 2;
			Game.mSplash.mTicks = 0;
			Game.mSplash.mX = 0;
			Game.mSplash.mY = 0;
			Game.mPd->sound->sampleplayer->setSample(Game.mSamplePlayer, Game.mResources.mAudio.mSampleIntroC);
			Game.mPd->sound->sampleplayer->play(Game.mSamplePlayer, 1, 1.0f);
		}
	}
	else if (Game.mSplash.mFlag == 2)
	{
		Game.mPd->graphics->drawBitmap(Game.mResources.mMenuFrame, Game.mSplash.mX, Game.mSplash.mY + SCREEN_HEIGHT, kBitmapUnflipped);
		Game.mPd->graphics->drawBitmap(Game.mResources.mMenuTitle, SCREEN_WIDTH / 2 - 128, 16, kBitmapUnflipped);
		Game.mPd->graphics->drawBitmap(Game.mResources.mMenuLogo, SCREEN_WIDTH/2 - 64 + 120, 75, kBitmapUnflipped);
		Game.mPd->graphics->drawBitmap(Game.mResources.mMenuLogo, SCREEN_WIDTH/2 - 64 - 120, 75, kBitmapUnflipped);
		Game.mSplash.mY -= 5.0f;
		if (Game.mSplash.mY < -SCREEN_HEIGHT)
		{
			Game.mMode = EMode_Menu;
			Game.mPd->sound->sampleplayer->setSample(Game.mSamplePlayer, Game.mResources.mAudio.mSampleIntroA);
			Game.mPd->sound->sampleplayer->play(Game.mSamplePlayer, 1, 1.0f);
		}
	}
}

void UpdateScoreboard()
{
	DrawGeneralBackground(0, 0, true);


	Game.mPd->graphics->setFont(Game.mResources.mFont);
	int x = SCREEN_WIDTH / 2;
	Game.mPd->graphics->fillRect(SCREEN_WIDTH / 2-120, 90, 240, 120, kColorWhite);
	Game.mPd->graphics->drawRect(SCREEN_WIDTH / 2-120, 90, 240, 120, kColorBlack);
	for (int i=0; i<3; i++)
	{
		int x = 130 + i * 70;
		DrawText(Game.mPd, MenuStrings[i+1], x, 100 , Game.mResources.mFont, true);
		for (int j=0; j<MAX_SCORES; j++)
		{
			int y = 120 + j * 15;
			char buffer[64];
			snprintf(buffer, 64, "%04d", Game.mScoreboard[i * MAX_SCORES + j]);
			DrawText(Game.mPd, buffer, x, y, Game.mResources.mFont, true);
		}
	}

	PDButtons current;
	PDButtons pushed;
	Game.mPd->system->getButtonState(&current, &pushed, NULL);
	if (pushed & kButtonA || pushed & kButtonB)
	{
		Game.mMode = EMode_Menu;
	}

}

void LoadResources()
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
	Game.mResources.mMenuFrame = loadImageAtPath(Game.mPd, "images/MenuFrame");
	Game.mResources.mMenuTitle = loadImageAtPath(Game.mPd, "images/Title");
	Game.mResources.mMenuLogo = loadImageAtPath(Game.mPd, "images/RadLogo");
	Game.mResources.mMarkerMenu = loadImageAtPath(Game.mPd, "images/MarkerMenu");

	Game.mResources.mStageDamage = (LCDBitmap**)malloc(4 * sizeof(LCDBitmap*));
	Game.mResources.mStageDamage[0] = loadImageAtPath(Game.mPd, "images/StageOveraly_Damage3");
	Game.mResources.mStageDamage[1] = loadImageAtPath(Game.mPd, "images/StageOveraly_Damage2");
	Game.mResources.mStageDamage[2] = loadImageAtPath(Game.mPd, "images/StageOveraly_Damage1");
	Game.mResources.mStageDamage[3] = loadImageAtPath(Game.mPd, "images/StageOveraly_Damage0");

	Game.mResources.mArrow = loadImageAtPath(Game.mPd, "images/Arrow");

	const char *outErr;
	Game.mResources.mFont = Game.mPd->graphics->loadFont("fonts/font-full-circle", &outErr);
	Game.mPd->graphics->setFont(Game.mResources.mFont);


	Game.mResources.mAudio.mSampleMenuSelect = CreateAudioSample(Game.mPd, AudioDataMenuSelect);
	Game.mResources.mAudio.mSampleMenuClick = CreateAudioSample(Game.mPd, AudioDataMenuClick);
	Game.mResources.mAudio.mSampleInGameGrab_Empty = CreateAudioSample(Game.mPd, AudioDataInGameGrabEmpty);
	Game.mResources.mAudio.mSampleInGameGrab_Full = CreateAudioSample(Game.mPd, AudioDataInGameGrabFull);
	Game.mResources.mAudio.mSampleInGameShoot = CreateAudioSample(Game.mPd, AudioDataInGameShoot);
	Game.mResources.mAudio.mSampleInMergeDone = CreateAudioSample(Game.mPd, AudioDataMergeDone);
	Game.mResources.mAudio.mSampleInMergeFail = CreateAudioSample(Game.mPd, AudioDataMergeFail);

	Game.mResources.mAudio.mSampleIntroA = CreateAudioSample(Game.mPd, AudioIntroA);
	Game.mResources.mAudio.mSampleIntroB = CreateAudioSample(Game.mPd, AudioIntroB);
	Game.mResources.mAudio.mSampleIntroC = CreateAudioSample(Game.mPd, AudioIntroC);

	Game.mPd->sound->sampleplayer->setSample(Game.mSamplePlayer, Game.mResources.mAudio.mSampleIntroA);
	Game.mPd->sound->sampleplayer->play(Game.mSamplePlayer, 1, 1.0f);
}

void InitGame(PlaydateAPI* pd) 
{
	Game.mPd = pd;
	Game.mSamplePlayer = Game.mPd->sound->sampleplayer->newPlayer();
	srand(pd->system->getSecondsSinceEpoch(NULL));
	LoadResources();
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
	Game.mPd->sound->sampleplayer->freePlayer(Game.mSamplePlayer);
	Game.mPd->graphics->freeBitmap(Game.mResources.mStageBackground);
	Game.mPd->graphics->freeBitmap(Game.mResources.mSocket);
	Game.mPd->graphics->freeBitmap(Game.mResources.mMenuBackground);
	Game.mPd->graphics->freeBitmap(Game.mResources.mMenuFrame);
	Game.mPd->graphics->freeBitmap(Game.mResources.mMenuTitle);
	Game.mPd->graphics->freeBitmap(Game.mResources.mMenuLogo);
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
	Game.mPd->graphics->clear(1);
	switch (Game.mMode)
	{
	case EMode_SplashScreen:
		UpdateSplashScreen();
		break;
	case EMode_Menu:
		UpdateMenu();
		break;
	case EMode_Scoreboard:
		UpdateScoreboard();
		break;
	case EMode_InGame:
		StageUpdate(&Stage);
		break;
	case EMode_Tutorial:
		StageUpdateTutorial(&Stage);
		break;
	default:
		break;
	}
	Game.mPd->sprite->updateAndDrawSprites();
}


