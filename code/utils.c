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

void DrawAnimatedSpriteRotated(PlaydateAPI* pd, SAnimatedSprite* animatedData, float x, float y, float angle)
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
	//pd->graphics->drawBitmap(animatedData->mBitmaps[animatedData->mCurrentFrame], x, y, animatedData->mFlipFlag);
	pd->graphics->drawRotatedBitmap(animatedData->mBitmaps[animatedData->mCurrentFrame], x, y, angle, 0.5f, 0.5f, 1.0f, 1.0f);
}

void DrawAnimatedSprite(PlaydateAPI* pd, SAnimatedSprite* animatedData, float x, float y)
{
	DrawAnimatedSpriteRotated(pd, animatedData, x, y, 0.0f);
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

//=============================================================================================================
// Sounds
AudioSample* CreateAudioSample(PlaydateAPI* pd, AudioNoteData* dataNotes, int numNotes)
{
	// This is fix for now
	const int sampleRate = 44100;

	float duration = 5.0;
	AudioNoteData* noteData = dataNotes;
	for (int i=0; i<numNotes; i++, noteData++)
	{
		duration += noteData->duration;
	}

	int numberSamples = sampleRate * duration;
	uint8_t* soundData = (uint8_t*)malloc(numberSamples);

	noteData = dataNotes;
	int t = 0;
	for (int i = 0; i < numNotes; i++, noteData++, t++)
	{
		int t = 0;
		int step = 0;
		int count = 0;
		float freq =  (float)noteData->mFrequency / sampleRate;
		for (int j = 0; j < numberSamples; j++)
		{
			switch (noteData->shape)
			{
			case EAudioShape_Sin:
				soundData[t] = (uint8_t)(noteData->volume*255 * 0.5 * (1 + sin(t * freq)));
				break;
			case EAudioShape_Noise:
			case EAudioShape_Square:
			case EAudioShape_Triangle:
			default:
				pd->system->error("Note not implemented");
				break;
			}

			/*
			if (step % 2 == 1)
			{
				soundData[i] = 0;
			}
			else
			{
				soundData[i] = amplitud;
			}

			count++;
			if (count >= width)
			{
				step++;
				count = 0;
			}
			*/
		}
	}

	AudioSample* sample = pd->sound->sample->newSampleFromData(soundData, kSound8bitMono, sampleRate, numberSamples);
	return sample;
}
