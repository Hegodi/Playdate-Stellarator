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

inline void ResetAnimationSprite(SAnimatedSprite* animatedSprite)
{
	animatedSprite->mCurrentFrame = 0;
	animatedSprite->mFrameCount = 0;
	animatedSprite->mHasfinished = false;
}


inline SAnimatedSprite CreateAnimationSprite(LCDBitmap** bitmaps, int numberFrames, int framesPerSprite)
{
	SAnimatedSprite animatedSprite;
	animatedSprite.mBitmaps = bitmaps;
	animatedSprite.mNumberFrames = numberFrames;
	animatedSprite.mFramesPerSprite = framesPerSprite;
	animatedSprite.mFlipFlag = kBitmapUnflipped;
	animatedSprite.mLoop = true;
	ResetAnimationSprite(&animatedSprite);
	return animatedSprite;
}

inline void DrawText(PlaydateAPI* pd, const char* str, float x, float y, LCDFont* font, bool centered)
{
	int strLength = strlen(str);
	if (font != NULL)
	{
		pd->graphics->setFont(font);
		if (centered)
		{
			int txtWidth = pd->graphics->getTextWidth(font, str, strLength, kASCIIEncoding, 0);
			pd->graphics->drawText(str, strLength, kASCIIEncoding, x - txtWidth / 2, y);
		}
	}
	else
	{
		pd->graphics->drawText(str, strLength, kASCIIEncoding, x, y);
	}
}

void DrawAnimatedSprite(PlaydateAPI* pd, SAnimatedSprite* animatedData, float x, float y);

#endif
