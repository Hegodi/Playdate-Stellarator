#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include "pd_api.h"
#include <stdbool.h>

float RandomFloat();
float Clamp(float value, float minValue, float maxValue);

typedef struct
{
	float x;
	float y;
} Vec2f;

float ModuleSqr(Vec2f const* const vec);
float Module(Vec2f const* const vec);
float Dot(Vec2f const* const v1, Vec2f const* const v2);
Vec2f Perpendicular(Vec2f const* const v);
LCDBitmap *loadImageAtPath(PlaydateAPI* pd,  const char *path);

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

void ResetAnimationSprite(SAnimatedSprite* animatedSprite);
SAnimatedSprite CreateAnimationSprite(LCDBitmap** bitmaps, int numberFrames, int framesPerSprite);

void DrawText(PlaydateAPI* pd, const char* str, float x, float y, LCDFont* font, bool centered);
void DrawAnimatedSprite(PlaydateAPI* pd, SAnimatedSprite* animatedData, float x, float y);

#endif
