
/**
 * @file main.c
 * @brief unit test
 * @author Takashi Kashiwagi
 * @date 2018/7/5
 * @details 
 * --
 * License Type <MIT License>
 * --
 * Copyright 2018 Takashi Kashiwagi
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included 
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979
#endif

#define DEF_AR_ODER (12u)
#include "wav/wav.h"
#include "LPC/LPAnalyzer.h"
#include "SPTK.h"
#include "multipulse/PulseSearch.h"

#define DEF_MAX_SAMPLE_RATE (192000u) /** 192KHz */
#define DEF_FRAME_LENGTH_MSEC (20u)   /** msec */

#define DEF_NUM_OF_FRAME_PER_SEC (1000u / DEF_FRAME_LENGTH_MSEC)
#define DEF_MAX_SAMPLES_PER_FRAME (DEF_MAX_SAMPLE_RATE / DEF_NUM_OF_FRAME_PER_SEC)

void WF(double xdata[], double alpha[], uint32_t u32SampleCnt, uint32_t ARorder, double gamma)
{
	/*-- var --*/
	double qdata[DEF_MAX_SAMPLES_PER_FRAME];
	double temp1, temp2, tgamma, atemp;

	/*-- cast --*/
	/*-- begin --*/
	tgamma = gamma;

	for (uint32_t i = 0; i < u32SampleCnt; i++)
	{
		temp1 = 0.0;
		temp2 = 0.0;
		gamma = tgamma;
		for (uint32_t j = 1; j <= ARorder; j++)
		{
			if (i == j - 1)
			{
				break;
			}
			atemp = alpha[j];
			temp1 = temp1 - atemp * qdata[i - j];
			temp2 = temp2 - gamma * atemp * qdata[i - j];
			gamma = gamma * tgamma;
		}
		qdata[i] = xdata[i] + temp2;
		xdata[i] = qdata[i] - temp1;
	}
}


static _Bool MultiPulseProcessing(const double dfpInputData[], uint32_t u32SampleCnt, double dfpPulses[], double dfpOutPutData[])
{
	double AutoCor[DEF_MAX_SAMPLES_PER_FRAME];
	double pWorkData[DEF_MAX_SAMPLES_PER_FRAME * 10];
	double dfpLPC[DEF_AR_ODER + 1]; /** 最初の 1.0 と16 */
	//double dfpLSP[DEF_AR_ODER + 1];
	double Corcor[DEF_MAX_SAMPLES_PER_FRAME];
	double ImpulseResponse[DEF_MAX_SAMPLES_PER_FRAME];
	double dfpTemp[DEF_MAX_SAMPLES_PER_FRAME];

	if (u32SampleCnt > DEF_MAX_SAMPLES_PER_FRAME)
	{
		return false;
	}

	/** Get LPC Value */
	if (CalcAutocorrelation(dfpInputData, u32SampleCnt, pWorkData, AutoCor) == false)
	{
		printf("[%s (%d)]CalcAutocorrelation NG\n", __FUNCTION__, __LINE__);
		return false;
	}

	if (LevinsonDurbinMethod(AutoCor, u32SampleCnt, pWorkData, dfpLPC, DEF_AR_ODER) == false)
	{
		printf("[%s (%d)]CalcAutocorrelation NG\n", __FUNCTION__, __LINE__);
		return false;
	}
	for(uint32_t i=0;i<u32SampleCnt;i++){
		dfpTemp[i] = dfpInputData[i];
	}
	//WF(dfpTemp, dfpLPC, u32SampleCnt, DEF_AR_ODER, 0.90);

	/** Get Pulses */
	if (GetImpulseResponse(dfpLPC, DEF_AR_ODER, u32SampleCnt, ImpulseResponse) == false)
	{
		printf("[%s (%d)]GetImpulseResponse NG\n", __FUNCTION__, __LINE__);
		return false;
	}
	if (CalcAutocorrelation(ImpulseResponse, u32SampleCnt, pWorkData, AutoCor) == false)
	{
		printf("[%s (%d)]CalcAutocorrelation NG\n", __FUNCTION__, __LINE__);
		return false;
	}
	if (CalcCrosscorrelation(ImpulseResponse, dfpTemp, u32SampleCnt, Corcor) == false)
	{
		printf("[%s (%d)]CalcCrosscorrelation NG\n", __FUNCTION__, __LINE__);
		return false;
	}
	if (PulseSearch(AutoCor, Corcor, u32SampleCnt, u32SampleCnt/8, dfpPulses) == false)
	{
		printf("[%s (%d)]PulseSearch NG\n", __FUNCTION__, __LINE__);
		return false;
	}
#if 0
	for(uint32_t i=0;i<u32SampleCnt/2;i++){
		dfpPulses[i] = (dfpPulses[2*i] + dfpPulses[2*i+1])/2;
	}
	for(uint32_t i=0;i<u32SampleCnt/2;i++){
		dfpPulses[i + (u32SampleCnt / 2)] = dfpPulses[i];
	}
#endif
	I_filter_2(dfpPulses, dfpLPC, dfpOutPutData, u32SampleCnt, DEF_AR_ODER);
	//dfpOutPutData[0] = 32767;
	return true;
}


static _Bool VoiceChangerEngine(const double dfpInputData[], uint32_t u32SampleCnt, double dfpPulses[], double dfpOutPutData[])
{
	double dfpDiff[DEF_MAX_SAMPLES_PER_FRAME];
	double dfpDiffPulses[DEF_MAX_SAMPLES_PER_FRAME];
	double dfpDiffOut[DEF_MAX_SAMPLES_PER_FRAME];

	MultiPulseProcessing(dfpInputData, u32SampleCnt, dfpPulses, dfpOutPutData);
	for(uint32_t i=0;i<u32SampleCnt;i++){
		dfpOutPutData[i] = dfpPulses[i];
	}
#if 0
	for(uint32_t i=0;i<u32SampleCnt;i++){
		dfpDiff[i] = dfpOutPutData[i] - dfpInputData[i];
	}
	MultiPulseProcessing(dfpDiff, u32SampleCnt, dfpDiffPulses, dfpDiffOut);
	
	for(uint32_t i=0;i<u32SampleCnt;i++){
		dfpOutPutData[i] -= dfpDiffOut[i];
	}

	for(uint32_t i=0;i<u32SampleCnt;i++){
		dfpOutPutData[i] -= dfpInputData[i];
	}
#endif
	return true;
}

static uint32_t SearchZeroCrossPosition(const int16_t pi16samples[], uint32_t u32SampleCnt){
	uint32_t u32ZeroCrossPosition = 0;

	for(uint32_t i=(u32SampleCnt-1);i>1;i--){
		if(pi16samples[i] == 0){
			u32ZeroCrossPosition = i;
			break;
		}
		if((pi16samples[i] * pi16samples[i-1]) < 0){
			u32ZeroCrossPosition = i;
			break;
		}
	}
	return u32ZeroCrossPosition;
}

int main(int argc, char *argv[])
{
	char szInputWavFile[256];
	char szOutputWavFile[2][256];

	for (uint32_t i = 1; i < argc;)
	{
		if (argv[i][0] == '-')
		{
			switch (argv[i][1])
			{
			case 'i':
				i++;
				strcpy(szInputWavFile, argv[i]);
				i++;
				printf("InputFile : <%s>\r\n", szInputWavFile);
				break;
			case 'o':
				i++;
				strcpy(szOutputWavFile[0], argv[i]);
				strcpy(szOutputWavFile[1], argv[i]);
				i++;
				strcat(szOutputWavFile[0], "_wav.wav");
				strcat(szOutputWavFile[1], "_pulse.wav");
				printf("OutputFile : <%s>\r\n", szOutputWavFile[0]);
				printf("OutputFile : <%s>\r\n", szOutputWavFile[1]);
				break;
			default:
				i++;
				break;
			}
		}
		else
		{
			i++;
		}
	}
	stFmtChunk_t stFmtChunk;

	if (WavFileGetFmtChunk(szInputWavFile, &stFmtChunk) == false)
	{
		printf("[%s (%d)] WavFileGetFmtChunk NG\n", __FUNCTION__, __LINE__);
		return 0;
	}

	printf("stFmtChunk.u16waveFormatType = %u\n", stFmtChunk.u16waveFormatType);
	printf("stFmtChunk.u16formatChannel  = %u\n", stFmtChunk.u16formatChannel);
	printf("stFmtChunk.u32samplesPerSec  = %u\n", stFmtChunk.u32samplesPerSec);
	printf("stFmtChunk.u32bytesPerSec    = %u\n", stFmtChunk.u32bytesPerSec);
	printf("stFmtChunk.u16blockSize      = %u\n", stFmtChunk.u16blockSize);
	printf("stFmtChunk.u16bitsPerSample  = %u\n", stFmtChunk.u16bitsPerSample);

	stWaveFile_t *outfp = WavFileWriteFmtChunk(szOutputWavFile[0], &stFmtChunk);
	if (outfp == NULL)
	{
		printf("[%s (%d)] WavFileGetFmtChunk NG\n", __FUNCTION__, __LINE__);
		return 0;
	}

	uint32_t u32ChunkSize = 0;
	stWaveFile_t *fp = WavFileSearchTopOfDataChunk(szInputWavFile, &u32ChunkSize);

	if (fp == NULL)
	{
		printf("[%s (%d)] WavFileGetFmtChunk NG\n", __FUNCTION__, __LINE__);
		return 0;
	}
	printf("[%s (%d)] u32ChunkSize = %u bytes\n", __FUNCTION__, __LINE__, u32ChunkSize);

	uint8_t u8Buffer2[96000 * 2 * 2];
	uint32_t u32ReadCnt = 0;
	double dfpInputData[DEF_MAX_SAMPLES_PER_FRAME] = {0};

	uint32_t u32BufferSize = stFmtChunk.u32samplesPerSec * stFmtChunk.u16formatChannel * stFmtChunk.u16bitsPerSample / 8;
	u32BufferSize /= DEF_NUM_OF_FRAME_PER_SEC; /** msec */

	for (;;)
	{

		if (u32ReadCnt < u32BufferSize)
		{
			uint8_t u8Buffer[96000 * 2];
			uint64_t br = WavFileGetPCMData(fp, &u32ChunkSize, u8Buffer, u32BufferSize);
			if (br == 0)
			{
				break;
			}
			memcpy(&u8Buffer2[u32ReadCnt], u8Buffer, br);

			u32ReadCnt += br;
		}
		else
		{
			uint32_t u32SampleCnt = u32ReadCnt / sizeof(uint16_t);
			int16_t *pi16samples = (int16_t *)u8Buffer2;
			uint32_t u32ZeroCrossPosition = SearchZeroCrossPosition(pi16samples, u32SampleCnt);
			if(u32ZeroCrossPosition != 0){
				printf("<%u/%u>\n", u32ZeroCrossPosition, u32SampleCnt);
				u32SampleCnt = u32ZeroCrossPosition;
			}

			/** Applied Windows Function */
			for (uint32_t i = 0; i < u32SampleCnt; i++)
			{
				dfpInputData[i] = pi16samples[i];
			}
			u32ReadCnt -= u32SampleCnt* sizeof(int16_t);
			memcpy(u8Buffer2, &u8Buffer2[u32SampleCnt * sizeof(int16_t)], u32ReadCnt);

			{	
				double dfpOutPutData[DEF_MAX_SAMPLES_PER_FRAME];
				double dfpPulses[DEF_MAX_SAMPLES_PER_FRAME];

				if (VoiceChangerEngine(dfpInputData, u32SampleCnt, dfpPulses, dfpOutPutData) == false)
				{
					printf("[%s (%d)] VoiceChangerEngine NG\n", __FUNCTION__, __LINE__);
					continue;
				}

				int16_t i16data[DEF_MAX_SAMPLES_PER_FRAME];
				for (uint32_t i = 0; i < u32SampleCnt; i++)
				{
					i16data[i] = (int16_t)dfpOutPutData[i];
				}
				WavFileWritePCMData(outfp, (uint8_t *)i16data, u32SampleCnt*sizeof(int16_t), &u32ChunkSize);
			}
		}
	}
	WavFileWriteClose(outfp, u32ChunkSize);
	WavFileClose(fp);
	return 0;
}
