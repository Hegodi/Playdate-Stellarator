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
