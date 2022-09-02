#include <stdlib.h>
#include <stdio.h>
#define _USE_MATH_DEFINES
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
AudioSample* _CreateAudioSample(PlaydateAPI* pd, AudioNoteData* dataNotes, int numNotes)
{
	// This is fix for now
	const uint32_t sampleRate = 44100;
	float const dt = 1.0f / sampleRate;

	float duration = 0.0;
	AudioNoteData* noteData = dataNotes;
	for (int i=0; i<numNotes; i++, noteData++)
	{
		duration += noteData->duration;
	}

	int numberSamples = sampleRate * duration;
	uint8_t* soundData = (uint8_t*)malloc(numberSamples*sizeof(uint8_t));

	noteData = dataNotes;
	float t = 0;
	int ind = 0;
	for (int i = 0; i < numNotes; i++, noteData++)
	{
		uint8_t lastRaw = 0;
		float angularFreq = 2 * M_PI * noteData->mFrequency;
		uint8_t vol = noteData->volume * 255;
		int noteSamples = noteData->duration * sampleRate;
		int sampleAttack = noteData->attack * noteSamples;
		float attackDelta = sampleAttack > 0 ? 1.0f / sampleAttack : 0.0f;
		int sampleFade = (1.0f - noteData->fade)  * noteSamples;
		float fadeDelta = sampleFade < noteSamples ? 1.0f / (noteSamples - sampleFade) : 0.0f;
		float modulator = 0.0f;

		if (sampleAttack == 0)
		{
			modulator = 1.0f;
		}


		for (int j = 0; j < noteSamples; j++, t+=dt, ind++)
		{
			float rawData = 0;
			switch (noteData->shape)
			{
			case EAudioShape_Sin:
				rawData = 0.5f*(1.0f + cos(t * angularFreq));
				break;
			case EAudioShape_NoiseSin:
			{
				int sign = sin(t * angularFreq) > 0.0f ? 1.0f : 0.0f;
				rawData = 0.5f * (1.0f + sin(t * angularFreq)) * rand() / (RAND_MAX + 1.0f);
				break;
			}
			case EAudioShape_NoiseSquare:
			{
				int sign = sin(t * angularFreq) > 0.0f ? 1.0f : 0.0f;
				rawData = sign * rand() / (RAND_MAX + 1.0f);
				break;
			}
			case EAudioShape_Square:
				rawData = sin(t * angularFreq) > 0.0f ? 1.0f : 0.0f;
				break;
			case EAudioShape_Noise:
				rawData = rand() / (RAND_MAX + 1.0f);
				break;
			default:
				pd->system->error("Note not implemented");
				break;
			}

			soundData[ind] = modulator *  vol * rawData;

			if (j < sampleAttack)	modulator += attackDelta;
			else if (j > sampleFade) modulator -= fadeDelta;

			if (ind >= numberSamples)
			{
				pd->system->error("ERROR creating sounds!!");
			}
		}
	}

	AudioSample* sample = pd->sound->sample->newSampleFromData(soundData, kSound8bitMono, sampleRate, numberSamples);
	//AudioSample* sample = pd->sound->sample->newSampleFromData(soundData, kSound8bitMono, sampleRate, numberSamples);
	return sample;
}
