
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

#define DEF_AR_ODER (16u)
#include "wav/wav.h"
#include "LPC/LPAnalyzer.h"
#include "SPTK.h"

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
	printf("stFmtChunk.u32samplesPerSec  = %lu\n", stFmtChunk.u32samplesPerSec);
	printf("stFmtChunk.u32bytesPerSec    = %lu\n", stFmtChunk.u32bytesPerSec);
	printf("stFmtChunk.u16blockSize      = %u\n", stFmtChunk.u16blockSize);
	printf("stFmtChunk.u16bitsPerSample  = %u\n", stFmtChunk.u16bitsPerSample);

	stWaveFile_t *outfp = WavFileWriteFmtChunk(szOutputWavFile[0], &stFmtChunk);
	if(outfp == NULL){
		printf("[%s (%d)] WavFileGetFmtChunk NG\n", __FUNCTION__, __LINE__);
		return 0;
	}
	stWaveFile_t *outfp2 = WavFileWriteFmtChunk(szOutputWavFile[1], &stFmtChunk);
	if(outfp2 == NULL){
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
	printf("[%s (%d)] u32ChunkSize = %lu bytes\n", __FUNCTION__, __LINE__, u32ChunkSize);

	
	for (;;)
	{
		uint8_t u8Buffer[96000 * 2];
		uint32_t u32BufferSize = stFmtChunk.u32samplesPerSec * stFmtChunk.u16formatChannel * stFmtChunk.u16bitsPerSample / 8;
		u32BufferSize /= 50; /** 100 msec */
		uint64_t br = WavFileGetPCMData(fp, &u32ChunkSize, u8Buffer, u32BufferSize);

		if (br == 0)
		{
			break;
		}
		//printf("[%s (%d)] %lu bytes read OK (Remain = %lu)\n", __FUNCTION__, __LINE__, br, u32ChunkSize);
		{
			uint32_t u32SampleCnt = br / sizeof(uint16_t);
			//printf("[%s (%d)] u32SampleCnt = %lu)\n", __FUNCTION__, __LINE__, u32SampleCnt);
			if(u32SampleCnt <= 3840){
				double AutoCross[3840];
				double pInputData[3840];
				double pWorkData[3840*10];
				double dfpLPC[DEF_AR_ODER];
				double dfpLSP[DEF_AR_ODER];
				double Corcor[3840];
				double ImpulseResponse[3840];
				int16_t *pi16 = u8Buffer;

				/** LPC alpha */
				for (uint32_t i = 0; i < u32SampleCnt; i++)
				{
					pInputData[i] = pi16[i];
				}
				CalcAutocorrelation(pInputData, u32SampleCnt, pWorkData, AutoCross);
#if 0
				for(uint32_t i=0;i<u32SampleCnt;i++){
					printf("%f ", AutoCross[i]);
				}
				printf("\n");
#endif
				LevinsonDurbinMethod(AutoCross, u32SampleCnt, pWorkData, dfpLPC, DEF_AR_ODER);
#if 0
				for(uint32_t i=0;i<DEF_AR_ODER;i++){
					printf("%f ", dfpLPC[i]);
				}
				printf("\n");
				lpc2lsp(dfpLPC, dfpLSP, DEF_AR_ODER, 128, 4, 1e-6);
				for(uint32_t i=0;i<DEF_AR_ODER;i++){
					printf("%f ", dfpLSP[i]);
				}
				printf("\n");
#endif
				if(GetImpulseResponse(dfpLPC, DEF_AR_ODER, u32SampleCnt, ImpulseResponse) == false){
					printf("[%s (%d)]GetImpulseResponse NG\n", __FUNCTION__, __LINE__);
					break;
				}
				CalcAutocorrelation(ImpulseResponse, u32SampleCnt, pWorkData, AutoCross);
#if 0
				for(uint32_t i=0;i<u32SampleCnt;i++){
					printf("%f ", ImpulseResponse[i]);
				}
				printf("\n");
#endif				
				if(CalcCrosscorrelation(ImpulseResponse, pInputData, u32SampleCnt, Corcor) == false){
					printf("[%s (%d)]GetImpulseResponse NG\n", __FUNCTION__, __LINE__);
					break;
				}
#if 0

				for(uint32_t i=0;i<u32SampleCnt;i++){
					printf("%f ", Corcor[i]);
				}
				printf("\n");
#endif
				double Pulses[3840];
				if(PulseSerch(AutoCross, Corcor, u32SampleCnt,  u32SampleCnt / 4, Pulses) == false){
					printf("[%s (%d)]GetImpulseResponse NG\n", __FUNCTION__, __LINE__);
					break;
				}
#if 0
				for(uint32_t i=0;i<u32SampleCnt;i++){
					printf("%f ", Pulses[i]);
				}
				printf("\n");
#endif

				double dfpOutPutData[3840];	
				I_filter(Pulses, dfpLPC, dfpOutPutData, u32SampleCnt, DEF_AR_ODER);
				
				int16_t i16data[3840];
				for(uint32_t i=0;i<u32SampleCnt;i++){
					i16data[i] = (int16_t)dfpOutPutData[i];
				}
				WavFileWritePCMData(outfp, i16data, u32SampleCnt*sizeof(int16_t), &u32ChunkSize);
				for(uint32_t i=0;i<u32SampleCnt;i++){
					i16data[i] = (int16_t)Pulses[i];
				}
				uint32_t dummy;
				WavFileWritePCMData(outfp2, i16data, u32SampleCnt*sizeof(int16_t), &dummy);
				

			}
		}
	}
	WavFileWriteClose(outfp, u32ChunkSize);
	WavFileWriteClose(outfp2, u32ChunkSize);
	WavFileClose(fp);
	return 0;
}
