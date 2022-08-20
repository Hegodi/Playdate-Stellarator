#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include "pd_api.h"
#include <stdbool.h>

inline float RandomFloat() { return rand() / (RAND_MAX + 1.0f); }
inline float Clamp(float value, float minValue, float maxValue)
{
	return value < minValue ? minValue : (value > maxValue ? maxValue : value);
}

extern LCDBitmap *loadImageAtPath(PlaydateAPI* pd,  const char *path);

typedef struct  
{
	LCDBitmap** mBitmaps;
	int mNumberFrames;
	int mCurrentFrame;
	int mFramesPerSprite;
	int mFrameCount;
	int mFlipFlag;
	bool mLoop;
	bool mHasfinished;
} SAnimatedSprite;

inline SAnimatedSprite CreateAnimationSprite(LCDBitmap** bitmaps, int numberFrames, int framesPerSprite)
{
	SAnimatedSprite animatedSprite;
	animatedSprite.mBitmaps = bitmaps;
	animatedSprite.mNumberFrames = numberFrames;
	animatedSprite.mFramesPerSprite = framesPerSprite;
	animatedSprite.mCurrentFrame = 0;
	animatedSprite.mFrameCount = 0;
	animatedSprite.mFlipFlag = kBitmapUnflipped;
	animatedSprite.mLoop = true;
	animatedSprite.mHasfinished = false;
	return animatedSprite;
}

extern void UpdateAnimatedSprite(PlaydateAPI* pd, LCDSprite* sprite, SAnimatedSprite* animatedData);

typedef struct
{
	float x;
	float y;
} Vec2f;

inline float ModuleSqr(Vec2f const* const vec)
{
	return vec->x * vec->x + vec->y * vec->y;
}

inline float Module(Vec2f const* const vec)
{
	return sqrt(ModuleSqr(vec));
}

inline float Dot(Vec2f const* const v1, Vec2f const* const v2)
{
	return  v1->x * v2->x + v1->y * v2->y;
}

inline Vec2f Perpendicular(Vec2f const* const v)
{
	Vec2f p;
	p.x = -v->y;
	p.y = v->x;
	return p;
}

#endif
