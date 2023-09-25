#ifndef STAGE_H
#define STAGE_H

#include <stdbool.h>
#include "utils.h"
#include "ball.h"

#define MAX_EXPLOSIONS_FX 3

typedef struct  
{
	int mLevel;
	int mMaxSlots;
	int mMaxBalls;
	int mSpawnMinBalls;
	int mSpawnDelayMin;
	int mSpawnDelayMax;

} SStageConfig;

typedef struct 
{
	int mLevel;
	int mCountdown;
	bool mIsPaused;
	bool mIsGameOver;
	SBall* mBalls;
	int mNumberBalls;
	int mMaxNumberBalls;
	int mScore;
	SBall** mBallsInSlots;
	SBall* mBallGrabbed;

	int mMaxSlots;
	int mSlotSelected;
	int mSpawnMinBalls;
	int mSpawnDelayMin;
	int mSpawnDelayDelta;

	unsigned int mTicks;
	unsigned int mTickNextSpawn;
	Vec2f mAimDirection;
	Vec2f mHookPos;
	Vec2f mAnchorPos;
	float mGunAngle;
	int  mIsGrabbing;
	SAnimatedSprite mGrabberSprite;
	unsigned int mNextId;

	SAnimatedSprite mExplosionsFX[MAX_EXPLOSIONS_FX];
	Vec2f mPosExplosions[MAX_EXPLOSIONS_FX];
	bool mExplosionsActive[MAX_EXPLOSIONS_FX];
	int mLastExplosionInd;


	int mTutorialStage;
		
} SStage;

void StageInit(SStage* stage, SStageConfig* config);
void StageClear(SStage* stage);
void StageUpdateTutorial(SStage* stage);
void StageUpdate(SStage* stage);
#endif