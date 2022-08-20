#ifndef BALL_H
#define BALL_H
#include "pd_api.h"
#include "utils.h"

typedef struct
{
	LCDSprite* mSprite;
	float mRadius;
	Vec2f mV;
	bool mUpdatePhysics;
	bool mCanMerge;
	
} SBall;

#endif