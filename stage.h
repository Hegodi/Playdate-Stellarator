#ifndef STAGE_H
#define STAGE_H

#include <stdbool.h>
#include "utils.h"
#include "Ball.h"

typedef struct 
{
	bool mIsGameOver;
	SBall* mBalls;
	int mNumberBalls;
	int mNumberBallsAllocated;
	int mScore;

	Vec2f mAimDirection;
	Vec2f mHookPos;
	Vec2f mAnchorPos;
	float mGunAngle;
	int  mIsGrabbing;
	SBall* mBalGrabbed;
	SAnimatedSprite mAtomSelectedFX;
	unsigned int mNextId;
		
} SStage;

extern void StageInit(SStage* stage);
extern void StageClear(SStage* stage);
extern void StageDraw(SStage* stage);
extern void StageUpdate(SStage* stage);
#endif