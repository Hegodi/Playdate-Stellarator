#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "utils.h"

LCDBitmap* loadImageAtPath(PlaydateAPI* pd, const char* path)
{
	const char* outErr = NULL;
	LCDBitmap* img = pd->graphics->loadBitmap(path, &outErr);
	if (outErr != NULL) {
		pd->system->logToConsole("Error loading image at path '%s': %s", path, outErr);
	}
	return img;
}

void DrawAnimatedSprite(PlaydateAPI* pd, SAnimatedSprite* animatedData, float x, float y)
{
	if (animatedData->mHasfinished)
	{
		return;
	}

	animatedData->mFrameCount++;
	if (animatedData->mFrameCount >= animatedData->mFramesPerSprite)
	{
		if (!animatedData->mLoop && animatedData->mCurrentFrame == animatedData->mNumberFrames - 1)
		{
			animatedData->mHasfinished = true;
		}
		else
		{
			animatedData->mCurrentFrame = (animatedData->mCurrentFrame + 1) % animatedData->mNumberFrames;
			animatedData->mFrameCount = 0;
		}
	}
	pd->graphics->drawBitmap(animatedData->mBitmaps[animatedData->mCurrentFrame], x, y, animatedData->mFlipFlag);
}

float RandomFloat() 
{ 
	return rand() / (RAND_MAX + 1.0f); 
}

float Clamp(float value, float minValue, float maxValue)
{
	return value < minValue ? minValue : (value > maxValue ? maxValue : value);
}

float ModuleSqr(Vec2f const* const vec)
{
	return vec->x * vec->x + vec->y * vec->y;
}

float Module(Vec2f const* const vec)
{
	return sqrt(ModuleSqr(vec));
}

float Dot(Vec2f const* const v1, Vec2f const* const v2)
{
	return  v1->x * v2->x + v1->y * v2->y;
}

Vec2f Perpendicular(Vec2f const* const v)
{
	Vec2f p;
	p.x = -v->y;
	p.y = v->x;
	return p;
}


void ResetAnimationSprite(SAnimatedSprite* animatedSprite)
{
	animatedSprite->mCurrentFrame = 0;
	animatedSprite->mFrameCount = 0;
	animatedSprite->mHasfinished = false;
}


SAnimatedSprite CreateAnimationSprite(LCDBitmap** bitmaps, int numberFrames, int framesPerSprite)
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

void DrawText(PlaydateAPI* pd, const char* str, float x, float y, LCDFont* font, bool centered)
{
	int strLength = strlen(str);
	if (font != NULL)
	{
		//pd->graphics->setFont(font);
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

