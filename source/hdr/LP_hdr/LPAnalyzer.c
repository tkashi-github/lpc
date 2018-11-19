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
 * @return void
 */
_Bool CalcAutocorrelation(const double InputData[], uint32_t u32NumOfSamples, double dfpWork[], double OutputData[])
{
	/*-- var --*/
	double temp;
	double sum;

	/*-- begin --*/
	if((InputData == NULL) ||
		(dfpWork == NULL) ||
		(OutputData == NULL)){
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


/*====================================*/
/*==     相互相関関数を計算する     ==*/
/*==        mcrossを少し改編        ==*/
/*====================================*/
/*== hdata   : インパルス応答(入力) ==*/
/*== xdata   : 原信号(入力)         ==*/
/*== XS      : 配列長(入力)         ==*/
/*== Rhx     : 自己相関関数(出力)   ==*/
/*====================================*/
//void C_COR_2(double *hdata, double *xdata, int XS, double *Rhx)
/**
 * @brief Calc Autocorrelation function
 * @param [in]  InputData[u32NumOfSamples] Input Impulse Response
 * @param [in]  InputSamples[u32NumOfSamples] Input Samples
 * @param [in]  u32NumOfSamples Number Of Samples
 * @param [out]  OutputData[u32NumOfSamples] Output Samples
 * @return void
 */
_Bool CalcCrosscorrelation(const double InputImpulse[], const double InputSamples[], uint32_t u32NumOfSamples, double OutputData[])
{
	/*-- var --*/
	double temp;

	/*-- begin --*/
	for (uint32_t i = 0; i < u32NumOfSamples; i++)
	{
		temp = 0.0;
		for (uint32_t n = i; n < u32NumOfSamples; n++)
		{
			temp += InputSamples[n] * InputImpulse[n - i];
		}
		OutputData[i] = temp / (double)u32NumOfSamples;
	}
}

/*======================================*/
/*==         塩原のプログラム         ==*/
/*== レビンソンアルゴリズム AIC無し   ==*/
/*==  Levinson Durbin  Method  ==*/
/*======================================*/
/*== Rxx     : 自己相関関数(入力)     ==*/
/*== XS      : 配列長(入力)           ==*/
/*== KP      : PARCOR係数(出力)       ==*/
/*== alpha   : 線形予測係数(出力)     ==*/
/*== ARorder : AR次数(入力)           ==*/
/*======================================*/
//void LevinsonDurbinMethod (double *Rxx, uint32_t u32NumOfSamples, double alpha[], uint32_t ARorder)
/**
 * @brief LevinsonDurbinMethod
 * @param [in]  AutoCor[u32NumOfSamples] Input Autocorrelation
 * @param [in]  u32NumOfSamples Number Of Samples
 * @param [in]  WorkBuffer[10 * u32NumOfSamples] Work Buffer
 * @param [out]  alpha[ARorder + 1] AR value
 * @param [in]  ARorder Number Of AR
 * @return void
 */
void LevinsonDurbinMethod (const double AutoCor[], uint32_t u32NumOfSamples, 
							double WorkBuffer[], double alpha[], uint32_t ARorder)
{
	/*-- var --*/
	int i, n;
	double *a, *b, *w, *u, *PARCOR;
	uint32_t WorkBufferSize = 2*u32NumOfSamples;

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
			for (i = 0; i <= ARorder; i++)
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
}

/*====================================*/
/*== procedure : IPR                ==*/
/*====================================*/
/*==         塩原のプログラム       ==*/
/*== AR係数からインパルス応答の生成 ==*/
/*====================================*/
/*== alpha   : 線形予測係数(入力)   ==*/
/*== ARorder : AR次数(入力)         ==*/
/*== XS      : 配列長(入力)         ==*/
/*== hdata   : インパルス応答(出力) ==*/
/*====================================*/
/**
 * @brief GetImpulseResponse
 * @param [in]  alpha[ARorder + 1] AR value
 * @param [in]  ARorder Number Of AR
 * @param [in]  u32NumOfSamples Number Of Samples
 * @param [out]  ImpulseResponse[u32NumOfSamples] Impulse Response
 * @return void
 */
void GetImpulseResponse(const double alpha[], uint32_t ARorder, uint32_t u32NumOfSamples, double ImpulseResponse[])
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
}

void I_filter(double *xdata,double *alpha,double *LP_E,
	      int XS,int ARorder)
{
  /*-- var --*/
  double  temp1;
  int  i,n;

  /*-- begin --*/
  for(n=0;n<XS;n++)
    {
      temp1 = 0.0;
      for(i=1;i<=ARorder;i++)
	{
	  if(n == i-1)  break;
	  temp1 = temp1 - alpha[i]* xdata[n-i];
	}
      LP_E[n] = xdata[n] - temp1;
    }  
}
