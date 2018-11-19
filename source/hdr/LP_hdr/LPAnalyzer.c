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
 * @param [out]  dfpWork[u32NumOfSamples] Output Samples
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
_Bool CalcCrosscorrelation(const double InputImpulse[], const double InputSamples[], uint32_t u32NumOfSamples, double dfpWork[], double OutputData[])
{
	/*-- var --*/
	double temp;
	int i, n;

	/*-- begin --*/
	for (i = 0; i < XS; i++)
	{
		temp = 0.0;
		for (n = i; n < XS; n++)
		{
			temp = temp + xdata[n] * hdata[n - i];
		}
		Rhx[i] = temp / (double)XS;
	}
}

void C_COR_2D(double hdata[16][16], double xdata[16][16], double Rhx[16][16])
{
	/*-- var --*/
	double temp;
	int i, j, m, n;

	/*-- begin --*/
	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 16; j++)
		{
			temp = 0.0;
			for (m = i; m < 16; m++)
			{
				for (n = j; n < 16; n++)
				{
					temp = temp + xdata[m][n] * hdata[m - i][n - j];
				}
			}
			Rhx[i][j] = temp / 256.0;
		}
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
void LDIM_A2(double *Rxx, int XS, double *alpha, int ARorder)
{
	/*-- var --*/
	int i, n;
	double *a, *b, *w, *u, *PARCOR;

	/*-- cast --*/
	a = (double *)malloc(2 * XS * sizeof(double));
	b = (double *)malloc(2 * XS * sizeof(double));
	w = (double *)malloc(2 * XS * sizeof(double));
	u = (double *)malloc(2 * XS * sizeof(double));
	PARCOR = (double *)malloc(2 * ARorder * sizeof(double));

	if ((a == NULL) || (b == NULL) || (w == NULL) || (u == NULL) ||
		(PARCOR == NULL))
	{
		printf("# LDIM2 hoge\n");
	}

	/*-- begin --*/

	/*-- Initialize --*/
	b[0] = 1.0;
	b[1] = 0.0;
	w[0] = *(Rxx + 1);
	u[0] = *(Rxx + 0);

	for (n = 0;; n++)
	{
		PARCOR[n + 1] = w[n] / u[n];
		a[0] = 1.0;
		for (i = 1; i <= n + 1; i++)
		{
			a[i] = b[i] - PARCOR[n + 1] * b[n + 1 - i];
		}
		a[n + 2] = 0.0;
		u[n + 1] = u[n] - PARCOR[n + 1] * w[n];

		if (n == ARorder - 1)
		{
			for (i = 0; i <= ARorder; i++)
			{
				*(alpha + i) = a[i];
			}
			break;
		}

		/*-- レビンソンアルゴリズムの最終段 --*/
		w[n + 1] = 0.0;
		for (i = 0; i <= n + 1; i++)
		{
			w[n + 1] += a[i] * Rxx[n + 2 - i];
		}
		for (i = 0; i < 2 * XS; i++)
		{
			b[i] = a[i];
		}
	}

	free(a);
	free(b);
	free(w);
	free(u);
	free(PARCOR);
} /*-- levin2 --*/

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
void IPR(double *alpha, int ARorder, int XS, double *hdata)
{
	/*-- var --*/
	int i, j;
	double temp;

	/*-- begin --*/
	hdata[0] = 1.0;

	for (i = 1; i < XS; i++)
	{
		temp = 0.0;
		for (j = 1; j <= i; j++)
		{
			if (j == ARorder + 1)
			{
				break;
			}
			temp = temp - alpha[j] * hdata[i - j];
		}
		hdata[i] = temp;
	}
}

/*==================================*/
/*== 名称:重み付け(1997/11/11)    ==*/
/*== 機能:重み付けを行なう関数    ==*/
/*==      Weighting Filter        ==*/
/*==       xdata = double         ==*/
/*==================================*/
/*== xdata   : 原信号(入力)       ==*/
/*== alpha   : 線形予測係数(入力) ==*/
/*== xwdata  : 重み付け信号(出力) ==*/
/*== XS      : 配列長(入力)       ==*/
/*== ARorder : AR次数(入力)       ==*/
/*== gamma   : 重み付け係数(入力) ==*/
/*==================================*/
void WF(double *xdata, double *alpha, double *xwdata,
		int XS, int ARorder, double gamma)
{
	/*-- var --*/
	double *qdata, temp1, temp2, tgamma;
	int i, j;

	/*-- cast --*/
	qdata = (double *)calloc(XS, sizeof(double));

	/*-- begin --*/
	tgamma = gamma;

	for (i = 0; i < XS; i++)
	{
		temp1 = 0.0;
		temp2 = 0.0;
		gamma = tgamma;
		for (j = 1; j <= ARorder; j++)
		{
			if (i == j - 1)
				break;
			temp1 = temp1 - alpha[j] * qdata[i - j];
			temp2 = temp2 - gamma * alpha[j] * qdata[i - j];
			gamma = gamma * tgamma;
		}
		qdata[i] = *(xdata + i) + temp2;
		*(xwdata + i) = qdata[i] - temp1;
	}
	free(qdata);
}

//めんどくさい
//まだなおしとチュウ
void WF_2D(double xdata[16][16], double *alpha_H, double *alpha_V,
		   double xwdata[16][16], int ARorder, double gamma)
{
	/*-- var --*/
	double qdataA[16][16], qdataB[16][16], qdataC[16][16];
	double temp1, temp2, temp3, temp4, tgamma;
	double wtempA[16][16], wtempB[16][16], wtempC[16][16];
	int i, j, k, l, tt;

	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 16; j++)
		{
			qdataA[i][j] = 0;
			qdataB[i][j] = 0;
			qdataC[i][j] = 0;
		}
	}

	/*-- begin --*/
	double gamma_H, gamma_V;
	tgamma = gamma;
	gamma_H = tgamma;
	gamma_V = tgamma;
	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 16; j++)
		{
			temp1 = 0.0;
			temp2 = 0.0;
			temp3 = 0.0;
			temp4 = 0.0;
			gamma_V = tgamma;
			gamma_H = tgamma;
			for (tt = 1; tt <= ARorder; tt++)
			{
				if (j == tt - 1)
				{
					break;
				}
				temp1 -= alpha_H[tt] * qdataA[i][j - tt];
				temp2 -= gamma_H * alpha_H[tt] * qdataA[i][j - tt];
				temp3 -= alpha_V[tt] * qdataB[j - tt][i];
				temp4 -= gamma_V * alpha_V[tt] * qdataB[j - tt][i];
				gamma_H = gamma_H * tgamma;
				gamma_V = gamma_V * tgamma;
			}
			qdataA[i][j] = xdata[i][j] + temp2;
			wtempA[i][j] = qdataA[i][j] - temp1;
			qdataB[j][i] = xdata[j][i] + temp4;
			wtempB[j][i] = qdataB[j][i] - temp3;
		}
	}

	tgamma = gamma;
	gamma_H = tgamma;
	gamma_V = tgamma;
	//printf("# gamma = %f\n",gamma);
	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 16; j++)
		{
			temp1 = 0.0;
			temp2 = 0.0;
			gamma_V = tgamma;
			for (k = 1; k <= ARorder; k++)
			{ //ここへんだ
				if (i - k < 0)
					break;
				gamma_H = tgamma;
				for (l = 1; l <= ARorder; l++)
				{
					if (j - l < 0)
						break;
					temp1 -= alpha_V[k] * alpha_H[l] * qdataC[i - k][j - l];
					temp2 -= gamma_V * gamma_H * alpha_V[k] * alpha_H[l] * qdataC[i - k][j - l];
					gamma_H = gamma_H * tgamma;
				}
				gamma_V = gamma_V * tgamma;
			}
			qdataC[i][j] = xdata[i][j] + temp2;
			wtempC[i][j] = qdataC[i][j] - temp1;
			xwdata[i][j] = wtempA[i][j] + wtempB[i][j] - wtempC[i][j];
			//xwdata[i][j] -= 2*xdata[i][j];
			//printf("# (%d,%d)=%f ?= %f\n",i,j,xwdata[i][j],xdata[i][j]);
		}
	}
}

void WF_2D_v2(double xdata[16][16], double *alpha_H, double *alpha_V,
			  double xwdata[16][16], int ARorder, double gamma)
{
	/*-- var --*/
	double qdataA[16][16], qdataB[16][16];
	double temp1, temp2, temp3, temp4, tgamma;
	double wtempA[16][16], wtempB[16][16];
	int i, j, tt;

	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 16; j++)
		{
			qdataA[i][j] = 0.0;
			qdataB[i][j] = 0.0;
		}
	}

	/*-- begin --*/
	double gamma_H, gamma_V;
	tgamma = gamma;
	//printf("# gamma = %f\n",gamma);
	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 16; j++)
		{
			temp1 = 0.0;
			temp2 = 0.0;
			gamma_H = tgamma;
			for (tt = 1; tt <= ARorder; tt++)
			{
				if (j == tt - 1)
				{
					break;
				}
				temp1 = temp1 - alpha_H[tt] * qdataA[i][j - tt];
				temp2 = temp2 - gamma_H * alpha_H[tt] * qdataA[i][j - tt];
				gamma_H = gamma_H * tgamma;
			}
			qdataA[i][j] = xdata[i][j] + temp2;
			wtempA[j][i] = qdataA[i][j] - temp1;
			//printf("# (%d,%d)=%f ?= %f\n",i,j,wtempA[j][i],xdata[i][j]);
		}
	}

	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 16; j++)
		{
			temp3 = 0.0;
			temp4 = 0.0;
			gamma_V = tgamma;
			for (tt = 1; tt <= ARorder; tt++)
			{
				if (j == tt - 1)
				{
					break;
				}
				temp3 = temp3 - alpha_V[tt] * qdataB[i][j - tt];
				temp4 = temp4 - gamma_V * alpha_V[tt] * qdataB[i][j - tt];
				gamma_V = gamma_V * tgamma;
			}
			qdataB[i][j] = wtempA[i][j] + temp4;
			wtempB[i][j] = qdataB[i][j] - temp3;
			xwdata[j][i] = wtempB[i][j];
			//printf("# (%d,%d)=%f ?= %f\n",j,i,xwdata[j][i],xdata[j][i]);
		}
	}
}

//何かオーバーフローしてる.なおしとチュウ
void LP_Synthesizer_2D(double *alpha_H, double *alpha_V, int ARorder,
					   double Pulses[16][16], double OUTdata[16][16])
{
	/*-- var --*/
	double temp1, temp2, tempA[16][16], tempB[16][16], tempC[16][16];
	int i, j, k, l, tt;

	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 16; j++)
		{
			tempA[i][j] = 0;
			tempB[i][j] = 0;
			tempC[i][j] = 0;
		}
	}

	/*-- begin --*/
	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 16; j++)
		{
			temp1 = 0.0;
			temp2 = 0.0;
			for (tt = 1; tt <= ARorder; tt++)
			{
				if (j - tt < 0)
				{
					break;
				}
				temp1 -= alpha_H[tt] * tempA[i][j - tt]; //水平方向
				temp2 -= alpha_V[tt] * tempB[j - tt][i]; //垂直方向
			}
			tempA[i][j] = Pulses[i][j] + temp1;
			tempB[j][i] = Pulses[j][i] + temp2;
			//printf("tempA(%d,%d) = %f : INdata = %f\n",i,j,tempA[i][j],INdata[i][j]);
			//printf("tempB(%d,%d) = %f : INdata = %f\n",j,i,tempB[j][i],INdata[j][i]);
			if (fabs(tempA[i][j]) > 128)
				printf("# Overflow!! tempA\n");
			if (fabs(tempB[i][j]) > 128)
				printf("# Overflow!! tempB\n");
		}
	}

	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 16; j++)
		{
			temp1 = 0.0;
			for (k = 1; k <= ARorder; k++)
			{ //ここへんだ
				if (i - k < 0)
					break;
				for (l = 1; l <= ARorder; l++)
				{
					if (j - l < 0)
						break;
					//temp1 -= alpha_V[k]*alpha_H[l]*tempC[i-k][j-l];
					temp1 += alpha_V[k] * alpha_H[l] * tempC[i - k][j - l];
				}
			}
			printf("# temp1 = %f\n", temp1);
			tempC[i][j] = Pulses[i][j] + temp1;
			if (fabs(tempC[i][j]) > 128)
				printf("# Overflow!! tempC\n"); //ここでOverflowしてる!!
			OUTdata[i][j] = tempA[i][j] + tempB[i][j] + tempC[i][j] - 2 * Pulses[i][j];
			//結果的に引くようになるのが正しい．よって書き方が複数ある．
			//間違えないように注意すること．
			//OUTdata[i][j] = tempA[i][j] + tempB[i][j] - tempC[i][j];
			//printf("# (%d,%d)=%f\n",i,j,OUTdata[i][j]);
			if (fabs(OUTdata[i][j]) > 128)
				printf("# Overflow!!  (%d,%d)=%f,[pulse=%f]\n", i, j, OUTdata[i][j], Pulses[i][j]);
		}
	}
}

void LP_Synthesizer_2D_v2(double *alpha_H, double *alpha_V, int ARorder,
						  double Pulses[16][16], double OUTdata[16][16])
{
	/*-- var --*/
	double temp1, temp2, tempA[16][16], tempB[16][16];
	int i, j, tt;

	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 16; j++)
		{
			tempA[i][j] = 0;
			tempB[i][j] = 0;
		}
	}

	/*-- begin --*/
	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 16; j++)
		{
			temp1 = 0.0;
			for (tt = 1; tt <= ARorder; tt++)
			{
				if (j - tt < 0)
				{
					break;
				}
				temp1 = temp1 - alpha_H[tt] * tempA[i][j - tt]; //水平方向
			}
			tempA[i][j] = Pulses[i][j] + temp1;
		}
	}

	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 16; j++)
		{
			temp2 = 0.0;
			for (tt = 1; tt <= ARorder; tt++)
			{
				if (j - tt < 0)
				{
					break;
				}
				temp2 = temp2 - alpha_V[tt] * tempB[j - tt][i]; //垂直方向
			}
			tempB[j][i] = tempA[j][i] + temp2;
			OUTdata[j][i] = tempB[j][i];
			if (fabs(OUTdata[j][i]) > 128)
				printf("# Overflow!!  (%d,%d)=%f,[pulse=%f]\n", j, i, OUTdata[j][i], Pulses[j][i]);
		}
	}
}
