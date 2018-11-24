/**
 * @file LPulsesnalyzer.c
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
 * FITNESS FOR A PulsesRTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 *
 * @Pulsesr Update:
 * - 2018/11/19: Takashi Kashiwagi: v0.1
 */
#include "PulseSearch.h"

_Bool PulseSearch(const double AutoCor[], double CrossCor[], uint32_t u32SampleCnt, uint32_t u32NumOfPulses, double Pulses[])
{
	/*-- var --*/
	uint32_t k;
	double rmax;
	double *tCrossCor = (double *)malloc(sizeof(double) * u32SampleCnt);
	_Bool *bIsPulseExist = (_Bool *)malloc(sizeof(_Bool) * u32SampleCnt); /* その位置にパルスが立っているか */

	if (tCrossCor == NULL)
	{
		return false;
	}
	if (bIsPulseExist == NULL)
	{
		free(tCrossCor);
		return false;
	}
	/*-- begin --*/
	for (uint32_t j = 0; j < u32SampleCnt; ++j)
	{
		tCrossCor[j] = CrossCor[j];
		bIsPulseExist[j] = false;
		Pulses[j] = 0;
	}

	k = 1;
	while (k <= u32NumOfPulses)
	{
		int32_t i32PulsePos = -1;
		rmax = 0.0;

		for (uint32_t j = 0; j < u32SampleCnt; ++j)
		{
			if (bIsPulseExist[j])
			{
				continue;
			}
			if (rmax < fabs(tCrossCor[j]))
			{

				i32PulsePos = j;
				rmax = fabs(tCrossCor[j]);
			}
		}
		if (i32PulsePos < 0)
		{
			printf("Not found\n");
			break;
		}

		
		bIsPulseExist[i32PulsePos] = true;

		


		Pulses[i32PulsePos] = tCrossCor[i32PulsePos] / AutoCor[0];
		//printf("Pulses[%d] = %f\n", k, Pulses[k]);
		for (uint32_t j = 0; j < u32SampleCnt; ++j)
		{
			tCrossCor[j] -= Pulses[i32PulsePos] * AutoCor[abs(j - i32PulsePos)];
		}
		++k;
	}

	free(tCrossCor);
	free(bIsPulseExist);
	return true;
}
