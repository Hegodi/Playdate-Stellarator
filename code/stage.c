#include "stage.h"
#include "game.h"
#include "utils.h"
#include <math.h>

#define  GAMEPLAY_XMIN 71
#define  GAMEPLAY_XMAX SCREEN_WIDTH - 7
#define  GAMEPLAY_YMIN 7
#define  GAMEPLAY_YMAX SCREEN_HEIGHT - 7

#define AIM_ANGLE_MAX 45.0f
#define SOCKET_POS_X 25
#define SOCKET_SIZE 40
#define HOOK_SPEED 20.0f
#define HOOK_RADIUS 16
#define SHOOT_SPEED 10.0f
#define ELASTIC_COEFFICIENT_WALL 0.9f
#define ELASTIC_COEFFICIENT_BALLS 0.8f
#define SPAWN_POINT_X 375
#define SPAWN_POINT_Y 32

#define ONE_MINUTE 1800

#define INVALID_ID 0

static int Points[] = { 0, 4, 8, 16, 32, 64, 128 };

void StageAddBall(SStage* stage, float x, float y, float vx, float vy, int energy);
void StageDeleteBall(SStage* stage, unsigned int id);
void StageUpdateInput(SStage* stage);
void StageUpdateAimDirection(SStage* stage);
void StageSetupAtom(SBall* ball, int energy);
void StageDraw(SStage* stage);

void StageInit(SStage* stage, SStageConfig* config)
{
	if (stage == NULL)
	{
		return;
	}

	stage->mTutorialStage = 0;

	stage->mIsGameOver = false;
	stage->mIsPaused = false;
	stage->mLevel = config->mLevel;
	stage->mCountdown = 3;
	stage->mTicks = 0;
	stage->mTickNextSpawn = 30;

	stage->mScore = 0;

	stage->mMaxNumberBalls = config->mMaxBalls;
	stage->mNumberBalls = 0;
	stage->mBalls = (SBall*)malloc(stage->mMaxNumberBalls * sizeof(SBall));
	for (int i = 0; i < stage->mMaxNumberBalls; i++)
	{
		stage->mBalls[i].mId = INVALID_ID;
	}

	stage->mGunAngle = 0.01f;
	stage->mIsGrabbing = false;
	stage->mHookPos = stage->mAnchorPos;

	stage->mMaxSlots = config->mMaxSlots;
	stage->mSlotSelected = stage->mMaxSlots / 2;
	stage->mBallsInSlots = (SBall**)malloc(stage->mMaxSlots * sizeof(SBall*));
	for (int i = 0; i < stage->mMaxSlots; i++)
	{
		stage->mBallsInSlots[i] = NULL;
	}
	stage->mBallGrabbed = NULL;

	stage->mSpawnDelayMin = config->mSpawnDelayMin;
	stage->mSpawnDelayDelta = config->mSpawnDelayMax - config->mSpawnDelayMin;
	stage->mSpawnMinBalls = config->mSpawnMinBalls;

	for (int i=0; i<MAX_EXPLOSIONS_FX; i++)
	{
		stage->mExplosionsFX[i] = CreateAnimationSprite(Game.mResources.mExplosionFX, 5, 2);
		stage->mExplosionsFX[i].mLoop = false;
		stage->mExplosionsActive[i] = false;
	}
	stage->mLastExplosionInd = 0;

	stage->mNextId = 1;

	stage->mGrabberSprite = CreateAnimationSprite(Game.mResources.mGrabberFX, 3, 1);
	StageUpdateAimDirection(stage);
}

void StageClear(SStage* stage)
{
	if (stage == NULL)
	{
		return;
	}

	stage->mNumberBalls = 0;
	stage->mMaxNumberBalls = 0;
	stage->mMaxSlots = 0;
	stage->mBallGrabbed = NULL;
	free(stage->mBalls);
	free(stage->mBallsInSlots);
	stage->mBalls = NULL;
	stage->mBallsInSlots = NULL;
}

void StageDraw(SStage* stage)
{
	if (stage == NULL || stage->mBalls == NULL)
	{
		return;
	}

	Game.mPd->graphics->drawBitmap(Game.mResources.mStageBackground, 0, 0, kBitmapUnflipped);
	int y = SCREEN_HEIGHT * 0.5f - stage->mMaxSlots * SOCKET_SIZE / 2;
	if (stage->mIsGrabbing)
	{
		Game.mPd->graphics->drawLine(stage->mAnchorPos.x, stage->mAnchorPos.y, stage->mHookPos.x, stage->mHookPos.y, 2, 0);
		DrawAnimatedSprite(Game.mPd, &stage->mGrabberSprite, stage->mHookPos.x, stage->mHookPos.y);
	}

	for (int i = 0; i < stage->mMaxSlots; i++, y+=SOCKET_SIZE)
	{
		if (stage->mSlotSelected == i)
		{
			Game.mPd->graphics->drawLine(stage->mAnchorPos.x, stage->mAnchorPos.y, stage->mAnchorPos.x + 64*stage->mAimDirection.x, stage->mAnchorPos.y + 64*stage->mAimDirection.y, 1, kColorBlack);
			Game.mPd->graphics->drawRotatedBitmap(Game.mResources.mArrow, stage->mAnchorPos.x, stage->mAnchorPos.y, stage->mGunAngle, 0.0f, 0.5f, 1.0f, 1.0f);
		}
		Game.mPd->graphics->drawBitmap(Game.mResources.mSocket, SOCKET_POS_X-16, y, kBitmapUnflipped);
	}


	static char buffer[128];

	SBall* ball = stage->mBalls;
	for (int i = 0; i < stage->mMaxNumberBalls; i++, ball++)
	{
		if (ball->mId == INVALID_ID)
		{
			continue;
		}

		if (ball->mCanMerge)
		{
			DrawAnimatedSpriteRotated(Game.mPd, &ball->mFXCanMerge, ball->mPos.x, ball->mPos.y, ball->mAngle);
		}
		Game.mPd->graphics->drawBitmap(Game.mResources.mBalls[ball->mEnergy - 1], ball->mPos.x - 16, ball->mPos.y - 16, kBitmapUnflipped);

#ifdef SHOW_DEBUG_TEXT
		snprintf(buffer, 128, "ID: %2d", ball->mId);
		Game.mPd->graphics->drawText(buffer, 128, kASCIIEncoding, ball->mPos.x - 16, ball->mPos.y-32);
#endif
	}

	for (int i = 0; i < MAX_EXPLOSIONS_FX; i++)
	{
		if (stage->mExplosionsActive[i])
		{
			DrawAnimatedSprite(Game.mPd, &stage->mExplosionsFX[i], stage->mPosExplosions[i].x, stage->mPosExplosions[i].y);
			stage->mExplosionsActive[i] = !stage->mExplosionsFX[i].mHasfinished;
		}
	}

	int secondsRaw = stage->mTicks / 30;
	int minutes = secondsRaw/ 60;
	int seconds = secondsRaw % 60;
	Game.mPd->graphics->setFont(Game.mResources.mFont);

	snprintf(buffer, 128, "%04d", stage->mScore);
	DrawText(Game.mPd, buffer, 27, 7, Game.mResources.mFont, true);
	if (stage->mCountdown < 0)
	{
		snprintf(buffer, 128, "%02d:%02d", minutes, seconds);
		DrawText(Game.mPd, buffer, 27, 20, Game.mResources.mFont, true);
	}

	snprintf(buffer, 128, "%02d/%02d", stage->mNumberBalls, (stage->mMaxNumberBalls-1));
	DrawText(Game.mPd, buffer, 27, SCREEN_HEIGHT - 20, Game.mResources.mFont, true);

	if (stage->mIsGameOver)
	{
		Game.mPd->graphics->drawBitmap(Game.mResources.mStageDamage[0], 0, 0, kBitmapUnflipped);
	}
	else
	{
		int ballsLeft = stage->mMaxNumberBalls - stage->mNumberBalls;
		if (ballsLeft < 4)
		{
			Game.mPd->graphics->drawBitmap(Game.mResources.mStageDamage[ballsLeft], 0, 0, kBitmapUnflipped);
		}
	}
}

void StageUpdateAimDirection(SStage* stage)
{
	float angleRad = stage->mGunAngle * 3.14159f / 180.0f;
	stage->mAimDirection.x = cosf(angleRad);
	stage->mAimDirection.y = sinf(angleRad);

	stage->mAnchorPos.x = SOCKET_POS_X;
	stage->mAnchorPos.y = SCREEN_HEIGHT * 0.5f - stage->mMaxSlots * SOCKET_SIZE / 2 + stage->mSlotSelected * SOCKET_SIZE + 16; 
}

void StageMergeBalls(SStage* stage, SBall* b1, SBall* b2)
{
	if (b1->mEnergy == 6)
	{
		StageDeleteBall(stage, b1->mId);
		StageDeleteBall(stage, b2->mId);
	}

	Vec2f vel = b1->mVel;
	StageSetupAtom(b2, b1->mEnergy + 1);
	StageDeleteBall(stage, b1->mId);
	b2->mVel.x += vel.x;
	b2->mVel.y += vel.y;
	b2->mVel.x *= 0.5f;
	b2->mVel.y *= 0.5f;
}


void StageUpdateSpawningBalls(SStage* stage)
{
	// Add new balls
	if (stage->mTickNextSpawn <= stage->mTicks)
	{
		if (stage->mNumberBalls == stage->mMaxNumberBalls-1)
		{
			stage->mIsGameOver = true;
			AddScore(stage->mScore, stage->mLevel-1);	// Tutorial is level 0, easy = 1
			Game.mPd->sound->sampleplayer->setSample(Game.mSamplePlayer, Game.mResources.mAudio.mSampleGameOver);
			Game.mPd->sound->sampleplayer->play(Game.mSamplePlayer, 1, 1.0f);
		}
		else
		{
			StageAddBall(stage, SPAWN_POINT_X, SPAWN_POINT_Y, -(1.0f + rand() / (RAND_MAX + 1.0f)), 5.0f, 1 + rand() % 2);

			if (stage->mNumberBalls < stage->mSpawnMinBalls)
			{
				stage->mTickNextSpawn = stage->mTicks + 30 + rand() % 30; // 1-2 seconds
			}
			else
			{
				stage->mTickNextSpawn = stage->mTicks + stage->mSpawnDelayMin + rand() % stage->mSpawnDelayDelta;
			}
		}
	}

}

void StageUpdatePhysics(SStage* stage)
{
	if (stage->mIsGrabbing > 0)
	{
		if (stage->mIsGrabbing == 1)
		{
			stage->mHookPos.x += stage->mAimDirection.x * HOOK_SPEED;
			stage->mHookPos.y += stage->mAimDirection.y * HOOK_SPEED;
			Vec2f dst = stage->mHookPos;
			dst.x -= stage->mAnchorPos.x;
			dst.y -= stage->mAnchorPos.y;
			if (ModuleSqr(&dst) > 1000000)
			{
				stage->mIsGrabbing = 2;
				Game.mPd->sound->sampleplayer->setSample(Game.mSamplePlayer, Game.mResources.mAudio.mSampleInGameGrab_Empty);
				Game.mPd->sound->sampleplayer->play(Game.mSamplePlayer, 1, 1.0f);
			}
			if (stage->mHookPos.x > GAMEPLAY_XMAX ||
				stage->mHookPos.y < GAMEPLAY_YMIN || stage->mHookPos.y > GAMEPLAY_YMAX)
			{
				stage->mIsGrabbing = 2;
				Game.mPd->sound->sampleplayer->setSample(Game.mSamplePlayer, Game.mResources.mAudio.mSampleInGameGrab_Empty);
				Game.mPd->sound->sampleplayer->play(Game.mSamplePlayer, 1, 1.0f);
			}
		}
		else if (stage->mIsGrabbing == 2)
		{
			stage->mHookPos.x -= stage->mAimDirection.x * HOOK_SPEED;
			stage->mHookPos.y -= stage->mAimDirection.y * HOOK_SPEED;
			Vec2f dst = stage->mHookPos;
			dst.x -= stage->mAnchorPos.x;
			dst.y -= stage->mAnchorPos.y;
			if (stage->mBallsInSlots[stage->mSlotSelected] != NULL)
			{
				stage->mBallsInSlots[stage->mSlotSelected]->mPos.x = stage->mHookPos.x;
				stage->mBallsInSlots[stage->mSlotSelected]->mPos.y = stage->mHookPos.y;
			}

			if (ModuleSqr(&dst) < 100)
			{
				stage->mBallsInSlots[stage->mSlotSelected] = stage->mBallGrabbed;
				stage->mBallGrabbed = NULL;
				stage->mIsGrabbing = 0;
				Game.mPd->sound->sampleplayer->stop(Game.mSamplePlayer);
			}
		}

		if (stage->mBallGrabbed == NULL)
		{
			SBall* ball = stage->mBalls;
			for (int i = 0; i < stage->mMaxNumberBalls; i++, ball++)
			{
				if (ball->mId == INVALID_ID || !ball->mUpdatePhysics)
				{
					continue;
				}

				float dx = ball->mPos.x - stage->mHookPos.x;
				float dy = ball->mPos.y - stage->mHookPos.y;
				float minDst = ball->mRadius + HOOK_RADIUS;
				if ((dx * dx + dy * dy) < minDst * minDst)
				{
					stage->mBallGrabbed = ball;
					ball->mUpdatePhysics = false;
					ball->mCanMerge = false;
					stage->mIsGrabbing = 2;
					Game.mPd->sound->sampleplayer->setSample(Game.mSamplePlayer, Game.mResources.mAudio.mSampleInGameGrab_Full);
					Game.mPd->sound->sampleplayer->play(Game.mSamplePlayer, 1, 1.0f);
					break;
				}
			}
		}
		else
		{
			stage->mBallGrabbed->mPos = stage->mHookPos;
		}

	}
	else
	{
		stage->mHookPos = stage->mAnchorPos;
		if (stage->mBallsInSlots[stage->mSlotSelected] != NULL)
		{
			stage->mBallsInSlots[stage->mSlotSelected]->mPos.x = stage->mHookPos.x;
			stage->mBallsInSlots[stage->mSlotSelected]->mPos.y = stage->mHookPos.y;
		}
	}

	// Update balls
	SBall* ball = stage->mBalls;
	for (int i = 0; i < stage->mMaxNumberBalls; i++, ball++)
	{
		if (ball->mId == INVALID_ID || !ball->mUpdatePhysics)
		{
			continue;
		}

		ball->mPos.x += ball->mVel.x;
		ball->mPos.y += ball->mVel.y;
	}

	// Resolve collisions
	for (int i = 0; i < stage->mMaxNumberBalls; i++)
	{
		SBall* b1 = &stage->mBalls[i];
		if (b1->mId == INVALID_ID || !b1->mUpdatePhysics)
		{
			continue;;
		}

		bool hasMerged = false;
		for (int j = i+1; j < stage->mNumberBalls; j++)
		{
			SBall* b2 = &stage->mBalls[j];
			if (b2->mId == INVALID_ID || !b2->mUpdatePhysics)
			{
				continue;;
			}
			Vec2f d;
			d.x = b2->mPos.x - b1->mPos.x;
			d.y = b2->mPos.y - b1->mPos.y;
			float dst2 = ModuleSqr(&d);
			float overlapDst = b1->mRadius + b2->mRadius;
			if (dst2 < overlapDst * overlapDst)
			{
				if ((b1->mCanMerge || b2->mCanMerge) && (b1->mEnergy == b2->mEnergy))
				{
					// Merge
					Game.mPd->sound->sampleplayer->setSample(Game.mSamplePlayer, Game.mResources.mAudio.mSampleInMergeDone);
					Game.mPd->sound->sampleplayer->play(Game.mSamplePlayer, 1, 1.0f);
					stage->mScore += Points[b1->mEnergy];
					int ind = (stage->mLastExplosionInd + 1) % MAX_EXPLOSIONS_FX;
					stage->mPosExplosions[ind].x = (b1->mPos.x + b2->mPos.x) * 0.5f;
					stage->mPosExplosions[ind].y = (b1->mPos.y + b2->mPos.y) * 0.5f;
					stage->mExplosionsActive[ind] = true;
					ResetAnimationSprite(&stage->mExplosionsFX[ind]);
					stage->mLastExplosionInd = ind;

					if (b1->mCanMerge)
					{
						StageMergeBalls(stage, b1, b2);
					}
					else if (b2->mCanMerge)
					{
						StageMergeBalls(stage, b2, b1);
					}
					hasMerged = true;
					break;
				}
				else
				{
					float dst = sqrt(dst2);
					Vec2f n = d;
					n.x /= dst;
					n.y /= dst;
					Vec2f t = Perpendicular(&n);
					float v1n = Dot(&b1->mVel, &n);
					float v2n = Dot(&b2->mVel, &n);
					float v1t = Dot(&b1->mVel, &t);
					float v2t = Dot(&b2->mVel, &t);
					if (b1->mCanMerge || b2->mCanMerge)
					{
						Game.mPd->sound->sampleplayer->setSample(Game.mSamplePlayer, Game.mResources.mAudio.mSampleInMergeFail);
						Game.mPd->sound->sampleplayer->play(Game.mSamplePlayer, 1, 1.0f);
					}
					b1->mCanMerge = false;
					b2->mCanMerge = false;

					// Extra safe to avoid duplicated collisions
					if (v2n - v1n < 0)
					{
						b1->mVel.x = (t.x * v1t + n.x * v2n) * ELASTIC_COEFFICIENT_BALLS;
						b1->mVel.y = (t.y * v1t + n.y * v2n) * ELASTIC_COEFFICIENT_BALLS;

						b2->mVel.x = (t.x * v2t + n.x * v1n) * ELASTIC_COEFFICIENT_BALLS;
						b2->mVel.y = (t.y * v2t + n.y * v1n) * ELASTIC_COEFFICIENT_BALLS;
					}
				}
			}
		}

		if (!hasMerged)
		{
			if (b1->mPos.x < GAMEPLAY_XMIN + b1->mRadius && b1->mVel.x < 0.0f)
			{
				b1->mPos.x = GAMEPLAY_XMIN + b1->mRadius;
				b1->mVel.x *= -ELASTIC_COEFFICIENT_WALL;
				if (b1->mCanMerge)
				{
					Game.mPd->sound->sampleplayer->setSample(Game.mSamplePlayer, Game.mResources.mAudio.mSampleInMergeFail);
					Game.mPd->sound->sampleplayer->play(Game.mSamplePlayer, 1, 1.0f);
				}
				b1->mCanMerge = false;
			}
			else if (b1->mPos.x > GAMEPLAY_XMAX - b1->mRadius && b1->mVel.x > 0.0f)
			{
				b1->mPos.x = GAMEPLAY_XMAX - b1->mRadius;;
				b1->mVel.x *= -ELASTIC_COEFFICIENT_WALL;
				if (b1->mCanMerge)
				{
					Game.mPd->sound->sampleplayer->setSample(Game.mSamplePlayer, Game.mResources.mAudio.mSampleInMergeFail);
					Game.mPd->sound->sampleplayer->play(Game.mSamplePlayer, 1, 1.0f);
				}
				b1->mCanMerge = false;
			}

			if (b1->mPos.y < GAMEPLAY_YMIN + b1->mRadius && b1->mVel.y < 0.0f)
			{
				b1->mPos.y = GAMEPLAY_YMIN + b1->mRadius;
				b1->mVel.y *= -ELASTIC_COEFFICIENT_WALL;
				if (b1->mCanMerge)
				{
					Game.mPd->sound->sampleplayer->setSample(Game.mSamplePlayer, Game.mResources.mAudio.mSampleInMergeFail);
					Game.mPd->sound->sampleplayer->play(Game.mSamplePlayer, 1, 1.0f);
				}
				b1->mCanMerge = false;
			}
			else if (b1->mPos.y > GAMEPLAY_YMAX - b1->mRadius && b1->mVel.y > 0.0f)
			{
				b1->mPos.y = GAMEPLAY_YMAX - b1->mRadius;
				b1->mVel.y *= -ELASTIC_COEFFICIENT_WALL;
				if (b1->mCanMerge)
				{
					Game.mPd->sound->sampleplayer->setSample(Game.mSamplePlayer, Game.mResources.mAudio.mSampleInMergeFail);
					Game.mPd->sound->sampleplayer->play(Game.mSamplePlayer, 1, 1.0f);
				}
				b1->mCanMerge = false;
			}
		}
	}
}

void StageUpdateTutorial(SStage* stage)
{
	const int x = SCREEN_WIDTH / 2;
	const int y = 10;
	PDButtons current;
	PDButtons pushed;
	Game.mPd->system->getButtonState(&current, &pushed, NULL);
	if (pushed & kButtonA)
	{
		Game.mPd->sound->sampleplayer->setSample(Game.mSamplePlayer, Game.mResources.mAudio.mSampleMenuClick);
		Game.mPd->sound->sampleplayer->play(Game.mSamplePlayer, 1, 1.0f);
		stage->mTutorialStage++;
		if (stage->mTutorialStage == 1)
		{
			StageAddBall(stage, SPAWN_POINT_X, SPAWN_POINT_Y, -(1.0f + rand() / (RAND_MAX + 1.0f)), 5.0f, 2);
		}
		else if (stage->mTutorialStage == 3)
		{
			StageAddBall(stage, SPAWN_POINT_X, SPAWN_POINT_Y, -(1.0f + rand() / (RAND_MAX + 1.0f)), 5.0f, 2);
		}

		if (stage->mTutorialStage > 5)
		{
			StageClear(stage);
			Game.mMode = EMode_Menu;
			return;
		}
	}

	switch(stage->mTutorialStage)
	{
	case 0:
		Game.mPd->graphics->fillRect(x-100, y, 200, 55, kColorWhite);
		Game.mPd->graphics->drawRect(x-100, y, 200, 55, kColorBlack);
		DrawText(Game.mPd, "Use the CRANK to aim", x, y+5, Game.mResources.mFont, true);
		DrawText(Game.mPd, "Pulse A to continue", x, y+35, Game.mResources.mFont, true);
		break;
	case 1:
		Game.mPd->graphics->fillRect(x-100, y, 200, 55, kColorWhite);
		Game.mPd->graphics->drawRect(x-100, y, 200, 55, kColorBlack);
		DrawText(Game.mPd, "Press the RIGHT button to", x, y+5, Game.mResources.mFont, true);
		DrawText(Game.mPd, "grab the atoms", x, y+17, Game.mResources.mFont, true);
		DrawText(Game.mPd, "Pulse A to continue", x, y+35, Game.mResources.mFont, true);
		break;
	case 2:
		Game.mPd->graphics->fillRect(x-100, y, 200, 70, kColorWhite);
		Game.mPd->graphics->drawRect(x-100, y, 200, 70, kColorBlack);
		DrawText(Game.mPd, "If you have a ball in the", x, y+5, Game.mResources.mFont, true);
		DrawText(Game.mPd, "socket, you can launch it", x, y+17, Game.mResources.mFont, true);
		DrawText(Game.mPd, "pressing RIGHT again", x, y+29, Game.mResources.mFont, true);
		DrawText(Game.mPd, "Pulse A to continue", x, y+50, Game.mResources.mFont, true);
		break;
	case 3:
		Game.mPd->graphics->fillRect(x-120, y, 240, 55, kColorWhite);
		Game.mPd->graphics->drawRect(x-120, y, 240, 55, kColorBlack);
		DrawText(Game.mPd, "You can change the current", x, y+5, Game.mResources.mFont, true);
		DrawText(Game.mPd, "socket with UP/DOWN buttons", x, y+17, Game.mResources.mFont, true);
		DrawText(Game.mPd, "Pulse A to continue", x, y+35, Game.mResources.mFont, true);
		break;
	case 4:
		Game.mPd->graphics->fillRect(x-120, y, 240, 100, kColorWhite);
		Game.mPd->graphics->drawRect(x-120, y, 240, 100, kColorBlack);
		DrawText(Game.mPd, "If the ball you launched hit", x, y+5, Game.mResources.mFont, true);
		DrawText(Game.mPd, "another ball of the same value", x, y+17, Game.mResources.mFont, true);
		DrawText(Game.mPd, "(before hitting anything else)", x, y+29, Game.mResources.mFont, true);
		DrawText(Game.mPd, "they will fuse and you will", x, y+41, Game.mResources.mFont, true);
		DrawText(Game.mPd, "get points", x, y+53, Game.mResources.mFont, true);
		DrawText(Game.mPd, "Pulse A to continue", x, y+80, Game.mResources.mFont, true);
		break;
	case 5:
		Game.mPd->graphics->fillRect(x-120, y, 240, 70, kColorWhite);
		Game.mPd->graphics->drawRect(x-120, y, 240, 70, kColorBlack);
		DrawText(Game.mPd, "Be careful, if you end up with", x, y+5, Game.mResources.mFont, true);
		DrawText(Game.mPd, "many balls in the reactor,", x, y+17, Game.mResources.mFont, true);
		DrawText(Game.mPd, "it will overheat.", x, y+29, Game.mResources.mFont, true);
		DrawText(Game.mPd, "Pulse A to return to main menu", x, y+50, Game.mResources.mFont, true);
		break;
	default:
		Game.mPd->graphics->fillRect(x-80, y, 160, 50, kColorWhite);
		Game.mPd->graphics->drawRect(x-80, y, 160, 50, kColorBlack);
	}

	stage->mTicks++;
	StageUpdateInput(stage);
	StageUpdatePhysics(stage);
	StageDraw(stage);
}

void StageUpdate(SStage* stage)
{
	if (stage->mIsPaused)
	{
		StageDraw(stage);
		int x = (SCREEN_WIDTH - GAMEPLAY_XMIN) / 2 + GAMEPLAY_XMIN;
		Game.mPd->graphics->fillRect(x - 100, 60, 200, 100, kColorWhite);
		Game.mPd->graphics->drawRect(x - 100, 60, 200, 100, kColorBlack);
		DrawText(Game.mPd, "PAUSE", x, 80, Game.mResources.mFont, true);
		DrawText(Game.mPd, "Press A to exit", x, 110, Game.mResources.mFont, true);
		DrawText(Game.mPd, "Press B to resume", x, 130, Game.mResources.mFont, true);
		PDButtons current;
		PDButtons pushed;
		Game.mPd->system->getButtonState(&current, &pushed, NULL);

		if (pushed & kButtonA)
		{
			Game.mPd->sound->sampleplayer->setSample(Game.mSamplePlayer, Game.mResources.mAudio.mSampleMenuClick);
			Game.mPd->sound->sampleplayer->play(Game.mSamplePlayer, 1, 1.0f);
			StageClear(stage);
			Game.mMode = EMode_Menu;
		}
		if (pushed & kButtonB)
		{
			Game.mPd->sound->sampleplayer->setSample(Game.mSamplePlayer, Game.mResources.mAudio.mSampleMenuClick);
			Game.mPd->sound->sampleplayer->play(Game.mSamplePlayer, 1, 1.0f);
			stage->mIsPaused = false;
		}
		return;
	}

	if (stage->mCountdown < 0)
	{
		if (stage->mIsGameOver)
		{
			StageDraw(stage);
			int x = (SCREEN_WIDTH - GAMEPLAY_XMIN) / 2 + GAMEPLAY_XMIN;
			Game.mPd->graphics->fillRect(x - 60, 90, 120, 60, kColorWhite);
			Game.mPd->graphics->drawRect(x - 60, 90, 120, 60, kColorBlack);
			static char buffer[128];
			DrawText(Game.mPd, "Game Over", x, 100, Game.mResources.mFont, true);
			snprintf(buffer, 128, "Score: %d", stage->mScore);
			DrawText(Game.mPd, buffer, x, 120, Game.mResources.mFont, true);
			PDButtons current;
			PDButtons pushed;
			Game.mPd->system->getButtonState(&current, &pushed, NULL);

			if (pushed & kButtonA || pushed & kButtonB)
			{
				Game.mPd->sound->sampleplayer->setSample(Game.mSamplePlayer, Game.mResources.mAudio.mSampleMenuClick);
				Game.mPd->sound->sampleplayer->play(Game.mSamplePlayer, 1, 1.0f);
				StageClear(stage);
				Game.mMode = EMode_Menu;
				return;
			}
		}
		else
		{
			stage->mTicks++;
			StageUpdateInput(stage);
			StageUpdateSpawningBalls(stage);
			StageUpdatePhysics(stage);
			StageDraw(stage);
		}
	}
	else
	{
		stage->mTicks++;
		StageDraw(stage);
		int x = (SCREEN_WIDTH - GAMEPLAY_XMIN) / 2 + GAMEPLAY_XMIN;
		Game.mPd->graphics->fillRect(x - 40, 90, 80, 40, kColorWhite);
		Game.mPd->graphics->drawRect(x - 40, 90, 80, 40, kColorBlack);
		if (stage->mCountdown > 0)
		{
			static char buffer[128];
			DrawText(Game.mPd, "Ready", x, 95, Game.mResources.mFont, true);
			snprintf(buffer, 128, "%d", stage->mCountdown);
			DrawText(Game.mPd, buffer, x, 110, Game.mResources.mFont, true);
		}
		else
		{
			DrawText(Game.mPd, "Go!", x, 100, Game.mResources.mFont, true);
		}

		if (stage->mTicks > 30)
		{
			if (stage->mCountdown == 2)
			{
				Game.mPd->sound->sampleplayer->setSample(Game.mSamplePlayer, Game.mResources.mAudio.mSampleCountdown3);
				Game.mPd->sound->sampleplayer->play(Game.mSamplePlayer, 1, 1.0f);
			}
			else if (stage->mCountdown == 1)
			{
				Game.mPd->sound->sampleplayer->setSample(Game.mSamplePlayer, Game.mResources.mAudio.mSampleCountdown2);
				Game.mPd->sound->sampleplayer->play(Game.mSamplePlayer, 1, 1.0f);
			}
			else if (stage->mCountdown == 0)
			{
				Game.mPd->sound->sampleplayer->setSample(Game.mSamplePlayer, Game.mResources.mAudio.mSampleCountdown1);
				Game.mPd->sound->sampleplayer->play(Game.mSamplePlayer, 1, 1.0f);
			}
			stage->mCountdown--;
			stage->mTicks = 0;
		}
	}

}

void StageUpdateInput(SStage* stage)
{
	PDButtons current;
	PDButtons pushed;
	Game.mPd->system->getButtonState(&current, &pushed, NULL);

	if (!stage->mIsGrabbing)
	{
		float deltaAngle = Game.mPd->system->getCrankChange();
		if (deltaAngle > 0.0f || deltaAngle < 0.0f)
		{
			stage->mGunAngle += deltaAngle;
			if (stage->mGunAngle > AIM_ANGLE_MAX) stage->mGunAngle = AIM_ANGLE_MAX;
			if (stage->mGunAngle < -AIM_ANGLE_MAX) stage->mGunAngle = -AIM_ANGLE_MAX;
			StageUpdateAimDirection(stage);
		}
	}

	if (pushed & kButtonRight)
	{
		if (stage->mIsGrabbing != 0)
		{
			if (stage->mBallGrabbed == NULL)
			{
				stage->mIsGrabbing = 0;
			}
			else if (stage->mBallGrabbed->mPos.x > GAMEPLAY_XMIN)
			{
				stage->mBallGrabbed->mUpdatePhysics = true;
				stage->mBallGrabbed->mVel.x *= 0.1f;
				stage->mBallGrabbed->mVel.y *= 0.1f;
				stage->mBallGrabbed = NULL;
				stage->mIsGrabbing = 0;
				Game.mPd->sound->sampleplayer->stop(Game.mSamplePlayer);
			}
		}
		else
		{
			if (stage->mBallsInSlots[stage->mSlotSelected] != NULL)
			{
				stage->mBallsInSlots[stage->mSlotSelected]->mUpdatePhysics = true;
				stage->mBallsInSlots[stage->mSlotSelected]->mVel.x = stage->mAimDirection.x * SHOOT_SPEED;
				stage->mBallsInSlots[stage->mSlotSelected]->mVel.y = stage->mAimDirection.y * SHOOT_SPEED;
				stage->mBallsInSlots[stage->mSlotSelected]->mCanMerge = true;
				stage->mBallsInSlots[stage->mSlotSelected]->mAngle = stage->mGunAngle + 45;
				stage->mBallsInSlots[stage->mSlotSelected] = NULL;
				Game.mPd->sound->sampleplayer->setSample(Game.mSamplePlayer, Game.mResources.mAudio.mSampleInGameShoot);
				Game.mPd->sound->sampleplayer->play(Game.mSamplePlayer, 1, 1.0f);
			}
			else
			{
				stage->mIsGrabbing = 1;
				Game.mPd->sound->sampleplayer->setSample(Game.mSamplePlayer, Game.mResources.mAudio.mSampleInGameGrab_Empty);
				Game.mPd->sound->sampleplayer->play(Game.mSamplePlayer, 1, 1.0f);
			}
		}
	}
	else if (pushed & kButtonUp && stage->mIsGrabbing == 0)
	{
		if (stage->mSlotSelected > 0)
		{
			Game.mPd->sound->sampleplayer->setSample(Game.mSamplePlayer, Game.mResources.mAudio.mSampleMenuSelect);
			Game.mPd->sound->sampleplayer->play(Game.mSamplePlayer, 1, 1.0f);
			stage->mSlotSelected--;
			StageUpdateAimDirection(stage);
		}
	}
	else if (pushed & kButtonDown && stage->mIsGrabbing == 0)
	{
		if (stage->mSlotSelected < stage->mMaxSlots - 1)
		{
			Game.mPd->sound->sampleplayer->setSample(Game.mSamplePlayer, Game.mResources.mAudio.mSampleMenuSelect);
			Game.mPd->sound->sampleplayer->play(Game.mSamplePlayer, 1, 1.0f);
			stage->mSlotSelected++;
			StageUpdateAimDirection(stage);
		}
	}
	else if (pushed & kButtonB && stage->mTutorialStage == 0)
	{
		Game.mPd->sound->sampleplayer->setSample(Game.mSamplePlayer, Game.mResources.mAudio.mSampleMenuClick);
		Game.mPd->sound->sampleplayer->play(Game.mSamplePlayer, 1, 1.0f);
		stage->mIsPaused = true;
	}
}

void StageAddBall(SStage* stage, float x , float y, float vx, float vy, int energy)
{
	stage->mNumberBalls++;
	if (stage->mNumberBalls >= stage->mMaxNumberBalls)
	{
		stage->mNumberBalls = stage->mMaxNumberBalls - 1;
		Game.mPd->system->error("ERROR: too many balls");
		return;
	}

	SBall* ballNew = NULL;
	for (int i=0; i<stage->mMaxNumberBalls; i++)
	{
		if (stage->mBalls[i].mId == INVALID_ID)
		{
			ballNew = &stage->mBalls[i];
			break;
		}
	}

	if (ballNew != NULL)
	{
		ballNew->mId = stage->mNextId;
		stage->mNextId++;
		ballNew->mPos.x = x;
		ballNew->mPos.y = y;
		ballNew->mVel.x = vx;
		ballNew->mVel.y = vy;
		ballNew->mUpdatePhysics = true;
		ballNew->mCanMerge = false;
		ballNew->mFXCanMerge = CreateAnimationSprite(Game.mResources.mAtomSelectedFX, 3, 2);
		StageSetupAtom(ballNew, energy);
	}
	else
	{
		//ERROR!!!! should never end up here
	}
	
}

void StageSetupAtom(SBall* ball, int energy)
{
	if (energy < 1 || energy > 6)
	{
		// ERROR!!
		Game.mPd->system->error("ERROR: ball with wrong enenrgy");
	}

	ball->mEnergy = energy;
	switch (energy)
	{
	case 1:
	case 2:
	case 3:
		ball->mRadius = 12.0f;
		break;
	case 4:
	case 5:
	case 6:
		ball->mRadius = 16.0f;
		break;
	default:
		break;
	}
}

void StageDeleteBall(SStage* stage, unsigned int id)
{
	for (int i=0; i<stage->mMaxNumberBalls; i++)
	{
		if (stage->mBalls[i].mId == id)
		{
			stage->mBalls[i].mId = INVALID_ID;
			break;
		}
	}
	stage->mNumberBalls--;
}
