#ifndef BALL_H
#define BALL_H
#include "pd_api.h"
#include "utils.h"

typedef struct
{
	float mRadius;
	Vec2f mVel;
	Vec2f mPos;
	float mAngle;
	bool mUpdatePhysics;
	bool mCanMerge;
	int mEnergy;
	SAnimatedSprite mFXCanMerge;
	unsigned int mId;
} SBall;

#endif