/**
 * @file LPAnalyzer.h
 * @brief TODO
 * @author Takashi Kashiwagi
 * @date 2018/11/19
 * @version     0.1
 * @details 
 * --
 * License Type <MIT License>
 * --
 * Copyright 2018 Takashi Kashiwagi
 * 
 * Permission is hereby granted, free of uint8_tge, to any person obtaining a 
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
 *
 * @par Update:
 * - 2018/11/19: Takashi Kashiwagi: v0.1
 */
#ifndef __cplusplus
#if __STDC_VERSION__ < 201112L
#error /** Only C11 */
#endif
#endif
#pragma once
#ifdef __cplusplus
extern "C"
{
#endif
#ifdef WIN_TEST
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#pragma pack(1)  
typedef struct
{
    uint8_t  u8chunkID[4];
    uint32_t u32chunkSize;
} stHdrChunk_t;

#pragma pack(1)  
typedef struct
{
    uint8_t  chunkFormType[4];
} stRiffChunk_t;

#pragma pack(1)  
typedef struct
{
    uint16_t u16waveFormatType;
    uint16_t u16formatChannel;
    uint32_t u32samplesPerSec;
    uint32_t u32bytesPerSec;
    uint16_t u16blockSize;
    uint16_t u16bitsPerSample;
} stFmtChunk_t;

#ifdef WIN_TEST
typedef FILE stWaveFile_t;
#elif defined(FATFS)
typedef FIL stWaveFile_t;
#else
#error Unkown Env
#endif

extern _Bool WavFileGetFmtChunk(const char szFilePath[], stFmtChunk_t *pstFmtChunk);
extern stWaveFile_t *WavFileSearchTopOfDataChunk(const char szFilePath[], uint32_t *pu32ChunkSize);
extern uint64_t WavFileGetPCMData(stWaveFile_t *fp, uint32_t *pu32RemainChunkSize, uint8_t pu8Buffer[], uint32_t u32BufferSize);
extern void WavFileClose(stWaveFile_t *fp);

#ifdef __cplusplus
}
#endif

