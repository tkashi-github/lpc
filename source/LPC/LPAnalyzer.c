/**
 * @file LPAnalyzer.c
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 *
 * @par Update:
 * - 2018/11/19: Takashi Kashiwagi: v0.1
 */
#include "LPAnalyzer.h"

/**
 * @brief Calc Autocorrelation function
 * @param [in]  InputData[u32NumOfSamples] Input Samples
 * @param [in]  u32NumOfSamples Number Of Samples
 * @param [in]  dfpWork[u32NumOfSamples] Work Buffer
 * @param [out]  OutputData[u32NumOfSamples] Output Samples
 * @return true OK
 * @return false NG
 */
_Bool CalcAutocorrelation(const double InputData[], uint32_t u32NumOfSamples, double dfpWork[], double OutputData[])
{
	/*-- var --*/
	double temp;
	double sum;

	/*-- begin --*/
	if ((InputData == NULL) ||
		(dfpWork == NULL) ||
		(OutputData == NULL))
	{
		return false;
	}
	sum = 0.0;
	/** TODO :Loop Unrolling */
	for (uint32_t i = 0; i < u32NumOfSamples; i++)
	{
		sum += InputData[i];
		dfpWork[i] = 0;
	}
	sum = sum / (double)u32NumOfSamples;

	/** TODO :Loop Unrolling */
	for (uint32_t i = 0; i < u32NumOfSamples; i++)
	{
		dfpWork[i] = (InputData[i] - sum);
	}

	/*-- calc auto-correlation function --*/
	/** TODO :Loop Unrolling */
	for (uint32_t i = 0; i < u32NumOfSamples; i++)
	{
		temp = 0.0;
		for (uint32_t n = i; n < u32NumOfSamples; n++)
		{
			temp += dfpWork[n] * dfpWork[n - i];
		}
		OutputData[i] = temp / (double)u32NumOfSamples;
	}

	return true;
}

/**
 * @brief Calc Crosscorrelation function
 * @param [in]  InputData[u32NumOfSamples] Input Impulse Response
 * @param [in]  InputSamples[u32NumOfSamples] Input Samples
 * @param [in]  u32NumOfSamples Number Of Samples
 * @param [out]  OutputData[u32NumOfSamples] Output Samples
 * @return true OK
 * @return false NG
 */
_Bool CalcCrosscorrelation(const double InputImpulse[], const double InputSamples[], uint32_t u32NumOfSamples, double OutputData[])
{
	/*-- var --*/
	double temp;

	/*-- begin --*/
	if ((InputImpulse == NULL) ||
		(InputSamples == NULL) ||
		(OutputData == NULL))
	{
		return false;
	}

	for (uint32_t i = 0; i < u32NumOfSamples; i++)
	{
		temp = 0.0;
		for (uint32_t n = i; n < u32NumOfSamples; n++)
		{
			temp += InputSamples[n] * InputImpulse[n - i];
		}
		OutputData[i] = temp / (double)u32NumOfSamples;
	}

	return true;
}

/**
 * @brief LevinsonDurbinMethod
 * @param [in]  AutoCor[u32NumOfSamples] Input Autocorrelation
 * @param [in]  u32NumOfSamples Number Of Samples
 * @param [in]  WorkBuffer[10 * u32NumOfSamples] Work Buffer
 * @param [out]  alpha[ARorder + 1] AR value
 * @param [in]  ARorder Number Of AR
 * @return true OK
 * @return false NG
 */
_Bool LevinsonDurbinMethod(const double AutoCor[], uint32_t u32NumOfSamples, double WorkBuffer[], double alpha[], uint32_t ARorder)
{
	/*-- var --*/
	double *a, *b, *w, *u, *PARCOR;
	uint32_t WorkBufferSize = 2 * u32NumOfSamples;

	/*-- cast --*/
	a = &WorkBuffer[0 * WorkBufferSize];
	b = &WorkBuffer[1 * WorkBufferSize];
	w = &WorkBuffer[2 * WorkBufferSize];
	u = &WorkBuffer[3 * WorkBufferSize];
	PARCOR = &WorkBuffer[4 * WorkBufferSize];

	/*-- begin --*/

	/*-- Initialize --*/
	b[0] = 1.0;
	b[1] = 0.0;
	w[0] = AutoCor[1];
	u[0] = AutoCor[0];

	for (uint32_t n = 0;; n++)
	{
		PARCOR[n + 1] = w[n] / u[n];
		a[0] = 1.0;
		for (uint32_t i = 1; i <= n + 1; i++)
		{
			a[i] = b[i] - PARCOR[n + 1] * b[n + 1 - i];
		}
		a[n + 2] = 0.0;
		u[n + 1] = u[n] - PARCOR[n + 1] * w[n];

		if (n == ARorder - 1)
		{
			for (uint32_t i = 0; i <= ARorder; i++)
			{
				alpha[i] = a[i];
			}
			break;
		}

		/*-- レビンソンアルゴリズムの最終段 --*/
		w[n + 1] = 0.0;
		for (uint32_t i = 0; i <= n + 1; i++)
		{
			w[n + 1] += a[i] * AutoCor[n + 2 - i];
		}
		for (uint32_t i = 0; i < 2 * u32NumOfSamples; i++)
		{
			b[i] = a[i];
		}
	}

	return true;
}

/**
 * @brief GetImpulseResponse
 * @param [in]  alpha[ARorder + 1] AR value
 * @param [in]  ARorder Number Of AR
 * @param [in]  u32NumOfSamples Number Of Samples
 * @param [out]  ImpulseResponse[u32NumOfSamples] Impulse Response
 * @return void
 */
_Bool GetImpulseResponse(const double alpha[], uint32_t ARorder, uint32_t u32NumOfSamples, double ImpulseResponse[])
{
	/*-- var --*/
	double temp;

	/*-- begin --*/
	ImpulseResponse[0] = 1.0;

	for (uint32_t i = 1; i < u32NumOfSamples; i++)
	{
		temp = 0.0;
		for (uint32_t j = 1; j <= i; j++)
		{
			if (j == (ARorder + 1))
			{
				break;
			}
			temp = temp - alpha[j] * ImpulseResponse[i - j];
		}
		ImpulseResponse[i] = temp;
	}

	return true;
}

void I_filter(const double dfpInData[], const double dfpAlpha[], double dfpOutput[], uint32_t u32SampleCnt, uint32_t ARorder)
{
	/*-- var --*/

	/*-- begin --*/
	for (uint32_t n = 0; n < u32SampleCnt; n++)
	{
		double temp1;
		temp1 = 0.0;
		for (uint32_t i = 1; i <= ARorder; i++)
		{
			if (n == i - 1)
			{
				break;
			}
			temp1 = temp1 - dfpAlpha[i] * dfpInData[n - i];
		}
		dfpOutput[n] = dfpInData[n] - temp1;
	}
}