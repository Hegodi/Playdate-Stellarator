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
#define SPAWN_POINT_X 375
#define SPAWN_POINT_Y 32

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

	stage->mIsGameOver = false;
	stage->mIsPaused = false;
	stage->mLevel = config->mLevel;
	stage->mCountdown = 3;
	stage->mTicks = 0;
	stage->mTickNextSpawn = 30;
	stage->mSlotSelected = 0;

	stage->mScore = 0;

	stage->mMaxNumberBalls = config->mMaxBalls;
	stage->mNumberBalls = 0;
	stage->mBalls = malloc(stage->mMaxNumberBalls * sizeof(SBall));

	stage->mGunAngle = 0.01f;
	stage->mIsGrabbing = false;
	stage->mHookPos = stage->mAnchorPos;

	stage->mMaxSlots = config->mMaxSlots;
	stage->mBallsInSlots = (SBall**)malloc(stage->mMaxSlots * sizeof(SBall*));
	for (int i = 0; i < stage->mMaxSlots; i++)
	{
		stage->mBallsInSlots[i] = NULL;
	}

	stage->mSpawnMinPeriod = config->mSpawnMinPeriod;
	stage->mSpawnDeltaPeriod = config->mSpawnMaxPeriod - config->mSpawnMinPeriod;
	stage->mMaxBallsReached = false;

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
	free(stage->mBalls);
	free(stage->mBallsInSlots);
}

void StageDraw(SStage* stage)
{
	if (stage == NULL)
	{
		return;
	}

	Game.mPd->graphics->drawBitmap(Game.mResources.mStageBackground, 0, 0, kBitmapUnflipped);
	int y = SCREEN_HEIGHT * 0.5f - stage->mMaxSlots * SOCKET_SIZE / 2;
	if (stage->mIsGrabbing)
	{
		Game.mPd->graphics->drawLine(stage->mAnchorPos.x, stage->mAnchorPos.y, stage->mHookPos.x, stage->mHookPos.y, 2, 0);
		DrawAnimatedSprite(Game.mPd, &stage->mGrabberSprite, stage->mHookPos.x - 16, stage->mHookPos.y - 16);
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


	SBall* ball = stage->mBalls;
	for (int i=0; i<stage->mNumberBalls; i++, ball++)
	{
		if (ball->mCanMerge)
		{
			DrawAnimatedSprite(Game.mPd, &ball->mFXCanMerge, ball->mPos.x - 32, ball->mPos.y - 32);
		}
		Game.mPd->graphics->drawBitmap(Game.mResources.mBalls[ball->mEnergy - 1], ball->mPos.x - 16, ball->mPos.y - 16, kBitmapUnflipped);
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

	static char buffer[128];
	//sprintf(buffer, "%s\n%02d:%02d\n%03d", stage->mLevel, minutes, seconds, stage->mScore);
	snprintf(buffer, 128, "%04d", stage->mScore);
	DrawText(Game.mPd, buffer, 27, 7, Game.mResources.mFont, true);
	if (stage->mCountdown < 0)
	{
		snprintf(buffer, 128, "%02d:%02d", minutes, seconds);
		DrawText(Game.mPd, buffer, 27, 20, Game.mResources.mFont, true);
	}

	snprintf(buffer, 128, "%02d/%02d", stage->mNumberBalls, stage->mMaxNumberBalls);
	DrawText(Game.mPd, buffer, 27, SCREEN_HEIGHT - 20, Game.mResources.mFont, true);

	if (stage->mIsGameOver)
	{
		Game.mPd->graphics->drawBitmap(Game.mResources.mStageDamage[0], 0, 0, kBitmapUnflipped);
	}
	else
	{
		int ballsLeft = stage->mMaxNumberBalls - stage->mNumberBalls;
		if (ballsLeft < 3)
		{
			Game.mPd->graphics->drawBitmap(Game.mResources.mStageDamage[ballsLeft + 1], 0, 0, kBitmapUnflipped);
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
			}

			if (stage->mHookPos.x > GAMEPLAY_XMAX ||
			    stage->mHookPos.y < GAMEPLAY_YMIN || stage->mHookPos.y > GAMEPLAY_YMAX)
			{
				stage->mIsGrabbing = 2;
			}
			else
			{
				SBall* ball = stage->mBalls;
				for (int i = 0; i < stage->mNumberBalls; i++, ball++)
				{
					if (!ball->mUpdatePhysics)
					{
						continue;
					}

					float dx = ball->mPos.x - stage->mHookPos.x;
					float dy = ball->mPos.y - stage->mHookPos.y;
					float minDst = ball->mRadius + HOOK_RADIUS;
					if ((dx * dx + dy * dy) < minDst * minDst)
					{
						stage->mBallsInSlots[stage->mSlotSelected] = ball;
						ball->mUpdatePhysics = false;
						ball->mCanMerge = false;
						stage->mIsGrabbing = 2;
						break;
					}
				}
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
				stage->mIsGrabbing = 0;
			}
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

	// Add new balls
	if (stage->mTickNextSpawn <= stage->mTicks)
	{
		if (stage->mNumberBalls == stage->mMaxNumberBalls)
		{
			stage->mIsGameOver = true;
			AddScore(stage->mScore, stage->mLevel);
		}
		else
		{
			StageAddBall(stage, SPAWN_POINT_X, SPAWN_POINT_Y, -(1.0f + rand() / (RAND_MAX + 1.0f)), 5.0f, 1 + rand() % 2);

			if (stage->mNumberBalls == stage->mMaxNumberBalls)
			{
				stage->mMaxBallsReached = true;
				stage->mTickNextSpawn = stage->mTicks + 5 * 30; // 5 seconds
			}
			else
			{
				stage->mTickNextSpawn = stage->mTicks + stage->mSpawnMinPeriod + rand() % stage->mSpawnDeltaPeriod;
			}
		}
	}

	// Update balls
	SBall* ball = stage->mBalls;
	for (int i = 0; i < stage->mNumberBalls; i++, ball++)
	{
		if (!ball->mUpdatePhysics)
		{
			continue;
		}

		ball->mPos.x += ball->mVel.x;
		ball->mPos.y += ball->mVel.y;
	}

	// Resolve collisions
	for (int i = 0; i < stage->mNumberBalls; i++)
	{
		SBall* b1 = &stage->mBalls[i];
		if (!b1->mUpdatePhysics)
		{
			continue;;
		}

		bool hasMerged = false;
		for (int j = i+1; j < stage->mNumberBalls; j++)
		{
			SBall* b2 = &stage->mBalls[j];
			if (!b2->mUpdatePhysics)
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
				if ( (b1->mCanMerge || b2->mCanMerge) && (b1->mEnergy == b2->mEnergy))
				{
					// Merge
					stage->mScore += Points[b1->mEnergy];
					int ind = (stage->mLastExplosionInd + 1) % MAX_EXPLOSIONS_FX;
					stage->mPosExplosions[ind].x = (b1->mPos.x + b2->mPos.x) * 0.5f - 32;
					stage->mPosExplosions[ind].y = (b1->mPos.y + b2->mPos.y) * 0.5f - 32;
					stage->mExplosionsActive[ind] = true;
					ResetAnimationSprite(&stage->mExplosionsFX[ind]);
					stage->mLastExplosionInd = ind;

					if (b1->mEnergy == 6)
					{
						StageDeleteBall(stage, b1->mId);
						StageDeleteBall(stage, b2->mId);
					}
					else
					{
						if (b1->mCanMerge)
						{
							Vec2f vel = b1->mVel;
							StageSetupAtom(b2, b1->mEnergy+1);
							StageDeleteBall(stage, b1->mId);
							b2->mVel.x += vel.x;
							b2->mVel.y += vel.y;
							b2->mVel.x *= 0.5f;
							b2->mVel.y *= 0.5f;
						}
						else if (b2->mCanMerge)
						{
							Vec2f vel = b2->mVel;
							StageSetupAtom(b1, b2->mEnergy+1);
							StageDeleteBall(stage, b2->mId);
							b1->mVel.x += vel.x;
							b1->mVel.y += vel.y;
							b1->mVel.x *= 0.5f;
							b1->mVel.y *= 0.5f;
						}
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
					b1->mCanMerge = false;
					b2->mCanMerge = false;

					// Extra safe to avoid duplicated collisions
					if (v2n - v1n < 0)
					{
						b1->mVel.x = t.x * v1t + n.x * v2n;
						b1->mVel.y = t.y * v1t + n.y * v2n;

						b2->mVel.x = t.x * v2t + n.x * v1n;
						b2->mVel.y = t.y * v2t + n.y * v1n;
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
				b1->mCanMerge = false;
			}
			else if (b1->mPos.x > GAMEPLAY_XMAX - b1->mRadius && b1->mVel.x > 0.0f)
			{
				b1->mPos.x = GAMEPLAY_XMAX - b1->mRadius;;
				b1->mVel.x *= -ELASTIC_COEFFICIENT_WALL;
				b1->mCanMerge = false;
			}

			if (b1->mPos.y < GAMEPLAY_YMIN + b1->mRadius && b1->mVel.y < 0.0f)
			{
				b1->mPos.y = GAMEPLAY_YMIN + b1->mRadius;
				b1->mVel.y *= -ELASTIC_COEFFICIENT_WALL;
				b1->mCanMerge = false;
			}
			else if (b1->mPos.y > GAMEPLAY_YMAX - b1->mRadius && b1->mVel.y > 0.0f)
			{
				b1->mPos.y = GAMEPLAY_YMAX - b1->mRadius;
				b1->mVel.y *= -ELASTIC_COEFFICIENT_WALL;
				b1->mCanMerge = false;
			}
		}
	}
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
			Game.mMode = EMode_Menu;
		}
		if (pushed & kButtonB)
		{
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
				Game.mMode = EMode_Menu;
			}
		}
		else
		{
			stage->mTicks++;
			StageUpdateInput(stage);
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
		float deltaAngle = 0.5f*Game.mPd->system->getCrankChange();
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
		if (stage->mIsGrabbing)
		{
			stage->mIsGrabbing = 0;
			if (stage->mBallsInSlots[stage->mSlotSelected] != NULL)
			{
				stage->mBallsInSlots[stage->mSlotSelected]->mUpdatePhysics = true;
				stage->mBallsInSlots[stage->mSlotSelected] = NULL;
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
				stage->mBallsInSlots[stage->mSlotSelected] = NULL;
			}
			else
			{
				stage->mIsGrabbing = 1;
			}
		}
	}
	else if (pushed & kButtonUp && stage->mIsGrabbing == 0)
	{
		if (stage->mSlotSelected > 0)
		{
			stage->mSlotSelected--;
			StageUpdateAimDirection(stage);
		}
	}
	else if (pushed & kButtonDown && stage->mIsGrabbing == 0)
	{
		if (stage->mSlotSelected < stage->mMaxSlots - 1)
		{
			stage->mSlotSelected++;
			StageUpdateAimDirection(stage);
		}
	}
	else if (pushed & kButtonB)
	{
		stage->mIsPaused = true;
	}
}

void StageAddBall(SStage* stage, float x , float y, float vx, float vy, int energy)
{
	stage->mNumberBalls++;
	/*
	Not needed: memory allocated on StageInit
	if (stage->mNumberBalls > stage->mNumberBallsAllocated)
	{
		stage->mNumberBallsAllocated += 8;
		stage->mBalls = realloc(stage->mBalls, stage->mNumberBallsAllocated * sizeof(SBall));
	}
	*/

	SBall* ballNew = &stage->mBalls[stage->mNumberBalls - 1];
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
	for (int i=0; i<stage->mNumberBalls; i++)
	{
		if (stage->mBalls[i].mId == id)
		{
			stage->mBalls[i] = stage->mBalls[stage->mNumberBalls - 1];
			break;
		}
	}
	stage->mNumberBalls--;
	stage->mMaxBallsReached = false;
}