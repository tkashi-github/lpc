
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

#define DEF_AR_ODER (4u)
#include "wav/wav.h"
#include "LPC/LPAnalyzer.h"

static void GetLSPPolynomial(
	int n,	 /* (i) qの配列長（線形予測次数の半分） */
	double *q, /* (i) 線スペクトル対のうち{奇数,偶数}番目の係数のみ
                           抜き出した配列 [0～(n-1)] */
	double *f  /* (o) 線スペクトル対 F_1(z), F_2(z) の係数 [0～n] */
)
{
	int i, j;

	f[0] = 1;
	for (i = 0; i < n; ++i)
	{
		f[i + 1] = -2.0 * q[i] * f[i] + ((i == 0) ? 0 : 2.0 * f[i - 1]);
		for (j = i; j > 0; --j)
		{
			f[j] = f[j] - 2.0 * q[i] * f[j - 1] + ((j == 1) ? 0 : f[j - 2]);
		}
	}
}

static double EvalChebyshev(
	double x, /* (i) 入力値(x = \cos(frequency)) */
	int n,	/* (i) fの最大有効インデックス（線形予測次数の半分） */
	double *f /* (i) Chebyshev多項式の係数 [0～n] */
)
/* --------------------------------------------------------------------
       Chebyshev方程式を計算する。
       C(x) = T_n(x) + f(1)T_{n-1}(x) + \cdots + f(n-1)T_1(x) + f(n)/2
       n = p / 2
       T_m(x) = \cos(m\omega)
       -------------------------------------------------------------------- */
{
	int i;
	double b0, b1, b2;

	b1 = 1.0;
	b2 = 0.0;
	for (i = n - 1; i >= 1; --i)
	{
		b0 = 2.0 * x * b1 - b2 + f[n - i];
		b2 = b1;
		b1 = b0;
	}
	b0 = x * b1 - b2 + f[n] / 2.0;

	return b0;
}
void ARtoLSP(
	int order,		   /* (i) 線形予測次数(偶数を想定) */
	const double ar[], /* (i) 線形予測係数(AR係数) [0～(order-1)] */
	double lsp[],	  /* (o) 線スペクトル対 [0～(order-1)], NULLも可 */
	double lsf[]	   /* (o) 線スペクトル周波数 [0～(order-1)], NULLも可 */
)
{
	const int GRID_POINTS = 200; /* 分割数 (default = 60) */
	const int DIVIDE_TIMES = 20; /* 分割回数 (default = 4) */
	/* 上の2つの値を大きくすることで、より正確な値を求められる */
	const int N = order / 2;

	int i, j;
	double xlow, ylow, xhigh, yhigh, xmid, ymid;
	double x, y;
	double xint;
	int nf = 0; /* スペクトルが見つかった本数 */

	double f1[DEF_AR_ODER * 2];
	double f2[DEF_AR_ODER * 2];

	if ((order & 0x1) != 0)
	{
		--order;
		if (lsp != NULL)
			lsp[order - 1] = 0.0;
		if (lsf != NULL)
			lsp[order - 1] = 0.0;
	}

	/* 多項式 F_1(z), F_2(z) の係数を求める。 */
	/* G.729 式(13),(14) 参照 */
	f1[0] = 1.0;
	f2[0] = 1.0;
	for (i = 0; i < N; ++i)
	{
		f1[i + 1] = ar[i] + ar[order - i - 1] - f1[i];
		f2[i + 1] = ar[i] - ar[order - i - 1] + f2[i];
	}

	/* 多項式のゼロ交差点を探す→ゼロ交差点が線スペクトルの位置 */
	xlow = 1.0; /* = cos(0) */
	ylow = EvalChebyshev(xlow, N, ((nf & 0x1) ? f2 : f1));

	for (j = 1; j < GRID_POINTS; ++j)
	{
		xhigh = xlow;
		yhigh = ylow;
		xlow = cos((M_PI * j) / GRID_POINTS);
		ylow = EvalChebyshev(xlow, N, ((nf & 0x1) ? f2 : f1));
		if (ylow * yhigh > 0.0)
			continue; /* xlow と xhigh の間にはゼロ交差点はない */

		/* ゼロ交差点の位置を詳しく調べる */
		for (i = 0; i < DIVIDE_TIMES; ++i)
		{
			xmid = (xhigh + xlow) / 2.0;
			ymid = EvalChebyshev(xmid, N, ((nf & 0x1) ? f2 : f1));
			if (ylow * ymid <= 0.0)
			{
				xhigh = xmid;
				yhigh = ymid;
			}
			else
			{
				xlow = xmid;
				ylow = ymid;
			}
		}
		/* ゼロ交差点の位置を線形補完する */
		x = xhigh - xlow;
		y = yhigh - ylow;
		xint = ((y == 0.0) ? xlow : xlow - ylow * (x / y));
		/* スペクトルを発見 */
		if (lsp != NULL)
			lsp[nf] = xint;
		if (lsf != NULL)
			lsf[nf] = acos(xint);
		++nf;
		if (nf == order)
			break; /* 必要な本数だけスペクトルが見つかったので終了 */
		/* 次の更新の準備 */
		xlow = xint;
		ylow = EvalChebyshev(xlow, N, ((nf & 0x1) ? f2 : f1));
	}

	if (nf < order)
	{
		/* order 本の線スペクトルが見つからなかった！ */
		/* printf("\n !!Not %d roots found in ARtoLSP()!!!\n", order); */
	}
}

void LSPtoAR(
	int order,			/* (i) 線形予測次数(偶数を想定) */
	const double lsp[], /* (i) 線スペクトル対 [0～(order-1)] */
	double *a			/* (o) 線形予測係数(AR係数) [0～(order-1)] */
)
{
	const int N = order / 2;

	int i;
	double f1[DEF_AR_ODER * 2];
	double f2[DEF_AR_ODER * 2];
	double q[DEF_AR_ODER * 2];

	if ((order & 0x1) != 0)
	{
		--order;
		a[order - 1] = 0.0;
	}

	/* F_1(z)の係数 */
	for (i = 0; i < N; ++i)
	{
		q[i] = lsp[i * 2];
	}
	GetLSPPolynomial(N, q, f1);
	/* F_2(z)の係数 */
	for (i = 0; i < N; ++i)
	{
		q[i] = lsp[i * 2 + 1];
	}
	GetLSPPolynomial(N, q, f2);

	for (i = N; i > 0; --i)
	{
		f1[i] = f1[i] + f1[i - 1];
		f2[i] = f2[i] - f2[i - 1];
	}

	for (i = 0; i < N; ++i)
	{
		a[i] = (f1[i + 1] + f2[i + 1]) / 2.0;
		a[order - i - 1] = (f1[i + 1] - f2[i + 1]) / 2.0;
	}
}

void LPtoLSP(double *IN_LP, double *OP_LSP)
{
	/*-- var --*/
	int i;
	double r[3], s[3], c;
	double tempA[5], Local_LSP[5];

	/*-- begin --*/
	r[1] = *(IN_LP + 1) - *(IN_LP + 4) + 1;
	r[2] = *(IN_LP + 2) - *(IN_LP + 3) + r[1];
	s[1] = *(IN_LP + 1) + *(IN_LP + 4) - 1;
	s[2] = *(IN_LP + 2) + *(IN_LP + 3) - s[1];

	c = r[2] / 2 - 1;

	tempA[2] = (-r[1] + sqrt(r[1] * r[1] - 4 * 2 * c)) / 4;
	tempA[4] = (-r[1] - sqrt(r[1] * r[1] - 4 * 2 * c)) / 4;

	c = s[2] / 2 - 1;

	tempA[1] = (-s[1] + sqrt(s[1] * s[1] - 4 * 2 * c)) / 4;
	tempA[3] = (-s[1] - sqrt(s[1] * s[1] - 4 * 2 * c)) / 4;

	for (i = 1; i <= 4; i++)
	{
		Local_LSP[i] = acos(tempA[i]);
	}
	for (i = 1; i <= 4; i++)
	{
		*(OP_LSP + i) = Local_LSP[i];
	}
}

void LSPtoLP(double *OP_LP, double *IN_LSP)
{
	int i;
	double tempLSP[5];
	double r[3], s[3];

	for (i = 1; i <= 4; i++)
	{
		tempLSP[i] = cos(IN_LSP[i]);
	}

	r[1] = -2 * (tempLSP[2] + tempLSP[4]);
	r[2] = 2 * (-2 * tempLSP[2] * tempLSP[2] - r[1] * tempLSP[2] + 1);
	s[1] = -2 * (tempLSP[1] + tempLSP[3]);
	s[2] = 2 * (-2 * tempLSP[1] * tempLSP[1] - s[1] * tempLSP[1] + 1);

	*(OP_LP + 1) = (r[1] + s[1]) / 2.0;
	*(OP_LP + 2) = (r[2] + s[2] - r[1] + s[1]) / 2.0;
	*(OP_LP + 3) = (s[2] - r[2] + r[1] + s[1]) / 2.0;
	*(OP_LP + 4) = (s[1] - r[1] + 2) / 2.0;
}

#if 0
void alflsp(int p, double alf[], double fq[])
{
	int i, j, km, kp, n;
	double x, opm[100], opp[100], acos();
	static double tbl[2600], eps = 0.0000001;
	chetbl(tbl, (p + 1) / 2); /* Chebyshev 変 換 表 作 成
*/
	n = p / 2;
	opp[0] = opm[0] = 1.0;
	for (i = 1; i <= n; i++)
	{
		opp[i] = alf[i] - alf[p + 1 - i] + opp[i - 1];
		opm[i] = alf[i] + alf[p + 1 - i] - opm[i - 1];
	}
	excheb(n, opp, opp, tbl);
	excheb(n, opm, opm, tbl);
	kp = n;
	km = n;
	j = 0;
	x = 1.0;
	for (i = 1; i <= p; i++)
	{
		if (j = 1 - j)
		{
			nrstep(opm, km, eps, &x);
			km--;
		}
		else
		{
			nrstep(opp, kp, eps, &x);
			kp--;
		}
		fq[i] = acos(fq[i]);
	}
}
#endif

static void get_alpha(int16_t pi16Buffer[], uint32_t u32SampleCnt)
{
	double *pInputData = malloc(sizeof(double) * u32SampleCnt);
	double *pAutoCorData = malloc(sizeof(double) * u32SampleCnt);
	double *pWorkData = malloc(sizeof(double) * u32SampleCnt * 10);
	double alpha[25] = {0};
	double alpha_2[25] = {0};
	double lsp[25] = {0};
	double lsf[25] = {0};
	for (uint32_t i = 0; i < u32SampleCnt; i++)
	{
		pInputData[i] = pi16Buffer[i];
	}
	CalcAutocorrelation(pInputData, u32SampleCnt, pWorkData, pAutoCorData);
	LevinsonDurbinMethod(pAutoCorData, u32SampleCnt, pWorkData, alpha, DEF_AR_ODER);
	for (uint32_t i = 0; i < DEF_AR_ODER; i++)
	{
		printf("%f ", alpha[i]);
	}
	printf("\n");
	ARtoLSP(DEF_AR_ODER, alpha, lsp, lsf);
	for (uint32_t i = 0; i < DEF_AR_ODER; i++)
	{
		printf("%f ", lsp[i]);
	}
	printf("\n");
	for (uint32_t i = 0; i < DEF_AR_ODER; i++)
	{
		printf("%f ", lsf[i]);
	}
	printf("\n");
	alpha_2[0] = 1;
	LSPtoAR(DEF_AR_ODER, lsp, alpha_2);
	for (uint32_t i = 0; i < DEF_AR_ODER; i++)
	{
		printf("%f ", alpha_2[i]);
	}
	printf("\n----\n");

	LPtoLSP(alpha, lsp);
	for (uint32_t i = 1; i <= DEF_AR_ODER; i++)
	{
		
		printf("%f ", lsp[i]);
	}
	printf("\n");
	alpha_2[0] = 1;
	LSPtoLP(alpha_2, lsp);
	for (uint32_t i = 0; i < DEF_AR_ODER; i++)
	{
		printf("%f ", alpha_2[i]);
	}
	printf("\n");
	for (uint32_t i = 1; i <= DEF_AR_ODER; i++)
	{
		lsp[i] = cos(lsp[i]);
	}
	LSPtoAR(DEF_AR_ODER, &lsp[1], alpha_2);
	for (uint32_t i = 0; i < DEF_AR_ODER; i++)
	{
		printf("%f ", alpha_2[i]);
	}
	printf("\n----\n");
}

int main(int argc, char *argv[])
{
	char szInputWavFile[256];
	char szOutputWavFile[256];

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
				strcpy(szOutputWavFile, argv[i]);
				i++;
				printf("OutputFile : <%s>\r\n", szOutputWavFile);
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

	uint32_t u32ChunkSize;
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
		printf("[%s (%d)] %lu bytes read OK (Remain = %lu)\n", __FUNCTION__, __LINE__, br, u32ChunkSize);
		get_alpha(u8Buffer, br);
	}

	WavFileClose(fp);
	return 0;
}
