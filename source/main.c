
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
#include "mimiclib.h"


static double EvalChebyshev(
    double x,   /* (i) 入力値(x = \cos(frequency)) */
    int n,      /* (i) fの最大有効インデックス（線形予測次数の半分） */
    double *f   /* (i) Chebyshev多項式の係数 [0～n] */
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
    for (i = n - 1; i >= 1; --i) {
        b0 = 2.0 * x * b1 - b2 + f[n - i];
        b2 = b1;
        b1 = b0;
    }
    b0 = x * b1 - b2 + f[n] / 2.0;

    return b0;
}


void LSPtoAR(
    int order,           /* (i) 線形予測次数(偶数を想定) */
    const double lsp[],  /* (i) 線スペクトル対 [0～(order-1)] */
    double *a            /* (o) 線形予測係数(AR係数) [0～(order-1)] */
)
{
    const int N = order / 2;
    
    int i;
    double *f1 = (double*)malloc(sizeof(double) * (N + 1));
    double *f2 = (double*)malloc(sizeof(double) * (N + 1));
    double *q = (double*)malloc(sizeof(double) * N);
    
    if ((order & 0x1) != 0) {
	--order;
	a[order - 1] = 0.0;
    }
        
    /* F_1(z)の係数 */
    for (i = 0; i < N; ++i) {
        q[i] = lsp[i * 2];
    }
    GetLSPPolynomial(N, q, f1);
    /* F_2(z)の係数 */
    for (i = 0; i < N; ++i) {
        q[i] = lsp[i * 2 + 1];
    }
    GetLSPPolynomial(N, q, f2);
    
    for (i = N; i > 0; --i) {
        f1[i] = f1[i] + f1[i - 1];
        f2[i] = f2[i] - f2[i - 1];
    }
    
    for (i = 0; i < N; ++i) {
        a[i] = (f1[i + 1] + f2[i + 1]) / 2.0;
        a[order - i - 1] = (f1[i + 1] - f2[i + 1]) / 2.0;
    }
    
    free(q);
    free(f2);
    free(f1);
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

	double *f1 = (double *)malloc(sizeof(double) * (N + 1));
	double *f2 = (double *)malloc(sizeof(double) * (N + 1));

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

	free(f2);
	free(f1);
}

void LSPAnalyser(double dfpInput[], uint32_t u32NumberOfSamples, uint32_t NumberOfPulses)
{

	/*== LPC分析 ==*/
	A_COR_2(dfpInput, XS, Rxx);
	LDIM_A2(Rxx, u32NumberOfSamples, alpha, ARorder);
	LPtoLSP(alpha, &tempLSP[0]);
	LSPtoLP(tAR, &tempLSP[0]);

	/*== インパルス応答の生成 ==*/
	IPR(tAR, ARorder, u32NumberOfSamples, InpulseResponse);

	/*== 重み付けフィルタ ==*/
	WF(xtemp, alpha, xwdata, u32NumberOfSamples, ARorder, gamma);
	WF(hdata, alpha, hwdata, u32NumberOfSamples, ARorder, gamma);

	/*== パルス探索 ==*/
	A_COR_2(hwdata, u32NumberOfSamples, Rhh);
	C_COR_2(hwdata, InpulseResponse, u32NumberOfSamples, Rhx);
	PS6_3(Rhx, Rhh, u32NumberOfSamples, NumberOfPulses, PL, PA);
}

int main(int argc, char *argv[])
{
	UnitTeset();
	return 0;
}
