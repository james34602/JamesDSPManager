#ifndef _JDSP_IMPL_H_
#define _JDSP_IMPL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#ifdef AOSP_SOONG_BUILD
#include <hardware/audio_effect.h>
#else
#include "essential.h"
#endif

#include "jdsp/jdsp_header.h"

typedef struct
{
	unsigned long long initializeForFirst;
	int32_t engineImpulseResponseHash;
	int32_t hashSlot[4];
	int mEnable;
	JamesDSPLib jdsp;
    float mSamplingRate;
    int formatFloatModeInt32Mode;
	char *stringEq;
	float *tempImpulseIncoming;
	float drcAtkRel[4];
	float boostingCon;
	int bbMaxGain;
	// Variables
	int numTime2Send, samplesInc, stringIndex;
	int16_t impChannels;
	int32_t impulseLengthActual, convolverNeedRefresh;
} EffectDSPMain;

void EffectDSPMainConstructor(EffectDSPMain *dspmain);
void EffectDSPMainDestructor(EffectDSPMain *dspmain);
int32_t EffectDSPMainCommand(EffectDSPMain *dspmain, uint32_t cmdCode, uint32_t cmdSize, void* pCmdData, uint32_t* replySize, void* pReplyData);
int32_t EffectDSPMainProcess(EffectDSPMain *dspmain, audio_buffer_t *in, audio_buffer_t *out);

#ifdef __cplusplus
}
#endif

#endif
