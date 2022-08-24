#ifndef STAGE_H
#define STAGE_H

#include <stdbool.h>
#include "utils.h"
#include "Ball.h"

typedef struct  
{
	int mMaxSlots;
	int mMaxBalls;
	int mSpawnMinPeriod;
	int mSpawnMaxPeriod;

} SStageConfig;

typedef struct 
{
	bool mIsGameOver;
	SBall* mBalls;
	int mNumberBalls;
	int mMaxNumberBalls;
	int mScore;
	SBall** mBallsInSlots;
	int mMaxSlots;
	int mSlotSelected;
	int mSpawnMinPeriod;
	int mSpawnDeltaPeriod;

	unsigned int mTicks;
	unsigned int mTickNextSpawn;
	Vec2f mAimDirection;
	Vec2f mHookPos;
	Vec2f mAnchorPos;
	float mGunAngle;
	int  mIsGrabbing;
	SAnimatedSprite mAtomSelectedFX;
	bool mMaxBallsReached;
	unsigned int mNextId;
		
} SStage;

void StageInit(SStage* stage, SStageConfig* config);
void StageClear(SStage* stage);
void StageDraw(SStage* stage);
void StageUpdate(SStage* stage);
#endif