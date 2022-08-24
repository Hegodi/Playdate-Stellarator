#include "stage.h"
#include "game.h"
#include "utils.h"
#include <math.h>

#define  GAMEPLAY_XMIN 56
#define  GAMEPLAY_XMAX SCREEN_WIDTH - 7
#define  GAMEPLAY_YMIN 7
#define  GAMEPLAY_YMAX SCREEN_HEIGHT - 7

#define AIM_ANGLE_MAX 60.0f
#define CANNON_LENGTH 40
#define GUN_POS_X 32
#define GUN_POS_Y SCREEN_HEIGHT / 2
#define HOOK_SPEED 20.0f
#define HOOK_RADIUS 20
#define SHOOT_SPEED 10.0f
#define ELASTIC_COEFFICIENT_WALL 0.9f

extern void StageAddBall(SStage* stage, float x, float y, float vx, float vy, int energy);
extern void StageDeleteBall(SStage* stage, unsigned int id);
extern void StageUpdateInput(SStage* stage);
extern void StageUpdateAimDirection(SStage* stage);
extern void StageSetupAtom(SBall* ball, int energy);

void StageInit(SStage* stage)
{
	if (stage == NULL)
	{
		return;
	}

	stage->mNumberBalls = 0;
	stage->mNumberBallsAllocated = 0;
	stage->mBalls = NULL;
	stage->mScore = 0;

	stage->mGunAngle = 0.01f;
	stage->mIsGrabbing = false;
	StageUpdateAimDirection(stage);
	stage->mHookPos = stage->mAnchorPos;
	stage->mBalGrabbed = NULL;

	StageAddBall(stage, 100.0f, SCREEN_HEIGHT*0.5f, 5.0f, 2.0f, 1);
	StageAddBall(stage, 300.0f, SCREEN_HEIGHT*0.5f, 3.0f, 1.0f, 1);
	StageAddBall(stage, 200.0f, SCREEN_HEIGHT*0.5f, 2.0f, 1.0f, 4);

	stage->mNextId = 1;
}

void StageClear(SStage* stage)
{
	if (stage == NULL)
	{
		return;
	}

	stage->mNumberBalls = 0;
	stage->mNumberBallsAllocated = 0.0001f;
	free(stage->mBalls);
}

void StageDraw(SStage* stage)
{
	if (stage == NULL)
	{
		return;
	}
	Game.mPd->graphics->clear(1);

	Game.mPd->graphics->drawBitmap(Game.mResources.mStageBackground, 0, 0, kBitmapUnflipped);
	Game.mPd->graphics->drawRotatedBitmap(Game.mResources.mGunFrame[0], GUN_POS_X, GUN_POS_Y, 90.0f, 0.5f, 0.5f, 1.0f, 1.0f, kBitmapUnflipped);
	Game.mPd->graphics->drawRotatedBitmap(Game.mResources.mGun, GUN_POS_X, GUN_POS_Y, stage->mGunAngle + 90.0f, 0.5f, 0.5f, 1.0f, 1.0f);

	if (stage->mIsGrabbing)
	{
		Game.mPd->graphics->drawLine(stage->mAnchorPos.x, stage->mAnchorPos.y, stage->mHookPos.x, stage->mHookPos.y, 2, 0);
		Game.mPd->graphics->drawEllipse(stage->mHookPos.x- HOOK_RADIUS/2, stage->mHookPos.y- HOOK_RADIUS/2, HOOK_RADIUS, HOOK_RADIUS, 2, 0, 360, 0);
	}

	SBall* ball = stage->mBalls;
	for (int i=0; i<stage->mNumberBalls; i++, ball++)
	{
		if (ball->mCanMerge)
		{
			DrawAnimatedSprite(Game.mPd, &ball->mFXCanMerge, ball->mPos.x - 32, ball->mPos.y - 32);
		}
		DrawAnimatedSprite(Game.mPd, &ball->mAnimatedSprite, ball->mPos.x - 16, ball->mPos.y - 16);
	}

	static char buffer[128];
	sprintf(buffer, "Score: %d", stage->mScore);
	Game.mPd->graphics->setFont(Game.mResources.mFont);
	Game.mPd->graphics->drawText(buffer, strlen(buffer), kASCIIEncoding, 10, 10);
}

void StageUpdateAimDirection(SStage* stage)
{
	float angleRad = stage->mGunAngle * 3.14159f / 180.0f;
	stage->mAimDirection.x = cosf(angleRad);
	stage->mAimDirection.y = sinf(angleRad);
	stage->mAnchorPos.x = GUN_POS_X + stage->mAimDirection.x * CANNON_LENGTH;
	stage->mAnchorPos.y = GUN_POS_Y + stage->mAimDirection.y * CANNON_LENGTH;
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
			if (ModuleSqr(&dst) > 300 * 300)
			{
				stage->mIsGrabbing = 2;
			}

			if (stage->mHookPos.x < GAMEPLAY_XMIN || stage->mHookPos.x > GAMEPLAY_XMAX ||
			    stage->mHookPos.y < GAMEPLAY_YMIN || stage->mHookPos.y > GAMEPLAY_YMAX)
			{
				stage->mIsGrabbing = 2;
			}
			else
			{
				SBall* ball = stage->mBalls;
				for (int i = 0; i < stage->mNumberBalls; i++, ball++)
				{
					float dx = ball->mPos.x - stage->mHookPos.x;
					float dy = ball->mPos.y - stage->mHookPos.y;
					float minDst = ball->mRadius + HOOK_RADIUS;
					if ((dx * dx + dy * dy) < minDst * minDst)
					{
						stage->mBalGrabbed = ball;
						ball->mUpdatePhysics = false;
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
			if (stage->mBalGrabbed != NULL)
			{
				stage->mBalGrabbed->mPos.x = stage->mHookPos.x;
				stage->mBalGrabbed->mPos.y = stage->mHookPos.y;
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
		if (stage->mBalGrabbed != NULL)
		{
			stage->mBalGrabbed->mPos.x = stage->mHookPos.x;
			stage->mBalGrabbed->mPos.y = stage->mHookPos.y;
		}
	}

	// Update position
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
					if (b1->mEnergy == 4)
					{
						StageDeleteBall(stage, b1->mId);
						StageDeleteBall(stage, b2->mId);
					}
					else
					{
						if (b1->mCanMerge)
						{
							StageSetupAtom(b2, 2 * b1->mEnergy);
							StageDeleteBall(stage, b1->mId);
						}
						else if (b2->mCanMerge)
						{
							StageSetupAtom(b1, 2 * b2->mEnergy);
							StageDeleteBall(stage, b2->mId);
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
	StageUpdateInput(stage);
	StageUpdatePhysics(stage);
}

void StageUpdateInput(SStage* stage)
{

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

	PDButtons current;
	PDButtons pushed;
	Game.mPd->system->getButtonState(&current, &pushed, NULL);
	if (current & kButtonRight)
	{
		if (stage->mIsGrabbing)
		{
			stage->mIsGrabbing = 0;
			if (stage->mBalGrabbed != NULL)
			{
				stage->mBalGrabbed->mUpdatePhysics = true;
				stage->mBalGrabbed = NULL;
			}
		}
		else
		{
			if (stage->mBalGrabbed != NULL)
			{
				stage->mBalGrabbed->mUpdatePhysics = true;
				stage->mBalGrabbed->mVel.x = stage->mAimDirection.x * SHOOT_SPEED;
				stage->mBalGrabbed->mVel.y = stage->mAimDirection.y * SHOOT_SPEED;
				stage->mBalGrabbed->mCanMerge = true;
				stage->mBalGrabbed = NULL;
			}
		}
	}
	else if (current & kButtonLeft)
	{
		if (!stage->mIsGrabbing && stage->mBalGrabbed == NULL)
		{
			stage->mIsGrabbing = 1;
		}
	}
	else if (current & kButtonUp)
	{
	}
	else if (current & kButtonDown)
	{
	}
	else
	{
	}

	if (pushed & kButtonA)
	{
	}
	else
	{
	}
}

void StageAddBall(SStage* stage, float x , float y, float vx, float vy, int energy)
{
	stage->mNumberBalls++;
	if (stage->mNumberBalls > stage->mNumberBallsAllocated)
	{
		stage->mNumberBallsAllocated += 8;
		stage->mBalls = realloc(stage->mBalls, stage->mNumberBallsAllocated * sizeof(SBall));
	}

	SBall* ballNew = &stage->mBalls[stage->mNumberBalls - 1];
	ballNew->mId = stage->mNextId;
	stage->mNextId++;
	ballNew->mPos.x = x;
	ballNew->mPos.y = y;
	ballNew->mVel.x = vx;
	ballNew->mVel.y = vy;
	ballNew->mUpdatePhysics = true;
	ballNew->mCanMerge = false;
	ballNew->mFXCanMerge = CreateAnimationSprite(Game.mResources.mAtomSelectedFX, 2, 2);
	StageSetupAtom(ballNew, energy);
	
}

void StageSetupAtom(SBall* ball, int energy)
{
	ball->mEnergy = energy;
	if (energy == 1)
	{
		ball->mRadius = 8.0f;
		ball->mAnimatedSprite = CreateAnimationSprite(Game.mResources.mAtom1, 4, 8);
	}
	else if (energy == 2)
	{
		ball->mRadius = 11.0f;
		ball->mAnimatedSprite = CreateAnimationSprite(Game.mResources.mAtom2, 4, 8);
	}
	else if (energy == 4)
	{
		ball->mRadius = 14.0f;
		ball->mAnimatedSprite = CreateAnimationSprite(Game.mResources.mAtom4, 4, 8);
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
}
