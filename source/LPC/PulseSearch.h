/*==================================================*/
/*== ヘッダを分割して管理することにします 2008/10 ==*/
/*==================================================*/
#include <stdio.h>
#include <math.h>

/*=============================*/
/*== procedure : SORT_PA     ==*/
/*=============================*/
/*==  パルス振幅の並べ替え   ==*/
/*=============================*/
/*== PL   : パルス位置(入力) ==*/
/*== PA   : パルス振幅(入力) ==*/
/*== NoP  : パルス数(入力)   ==*/
/*== XS   : 配列長(入力)     ==*/
/*== S_PA : パルス振幅(出力) ==*/
/*=============================*/
void SORT_PA_dPA(int *PL, double *PA, int NoP, int XS, double *S_PA)
{
	/*-- var --*/
	int i, j;
	double *temp;
	temp = (double *)calloc(XS, sizeof(double));

	/*-- begin --*/

	for (i = 1; i <= NoP; i++)
	{
		if (*(PA + i) > 127.0)
		{
			//printf("clipping : %f -> 127\n",*(PA+i));
			*(PA + i) = 127.0;
		}
		else if (*(PA + i) < -128.0)
		{
			//printf("clipping : %f -> -128\n",*(PA+i));
			*(PA + i) = -128.0;
		}
		temp[*(PL + i)] = *(PA + i);
	}
	j = 1;
	for (i = 0; i < XS; i++)
	{
		if (temp[i] != 0.0)
		{
			*(S_PA + j) = temp[i];
			j++;
		}
	}
	free(temp);
}

//2D対応
void SORT_PA_dPA_2D(int PL_X[256], int PL_Y[256],
					double PA[256], int NoP, double *S_PA)
{
	/*-- var --*/
	int i, j, tt;
	double temp[16][16];

	/*-- begin --*/
	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 16; j++)
		{
			temp[i][j] = 0.0;
		}
	}

	for (i = 1; i <= NoP; i++)
	{
		// printf("# hoge %d:%d,%d\n",i,PL_X[i],PL_Y[i]);
		if (PA[i] > 127.0)
		{
			//printf("clipping : %f -> 127\n",*(PA+i));
			PA[i] = 127.0;
		}
		else if (PA[i] < -128.0)
		{
			//printf("clipping : %f -> -128\n",*(PA+i));
			PA[i] = -128.0;
		}
		temp[PL_Y[i]][PL_X[i]] = PA[i];
	}

	tt = 1;
	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 16; j++)
		{
			if (temp[i][j] != 0.0)
			{
				*(S_PA + tt) = temp[i][j];
				tt++;
			}
		}
	}
	//printf("# sort tt= %d\n",tt-1);
}

/*==============================*/
/*== procedure : PS2          ==*/
/*==============================*/
/*==  パルス探索(相関法)(H17) ==*/
/*==     古川のプログラム     ==*/
/*==============================*/
/*== Rhx : 相互相関関数(入力) ==*/
/*== Rhh : 自己相関関数(入力) ==*/
/*== XS  : 配列長(入力)       ==*/
/*== NoP : パルス数(入力)     ==*/
/*== PL  : パルス位置(出力)   ==*/
/*== PA  : パルス振幅(出力)   ==*/
/*==============================*/
void PS2(double *Rhx, double *Rhh, int XS, int NoP, int *PL, double *PA)
{
	/*-- var --*/
	int j, k, pool;
	double rmax;
	double tRhx[XS], tRhh[XS];
	int isExcited[XS]; /* その位置にパルスが立っているか */

	/*-- begin --*/
	for (j = 0; j < XS; ++j)
	{
		tRhx[j] = *(Rhx + j);
		tRhh[j] = *(Rhh + j);
		isExcited[j] = 0;
	}

	k = 1;
	for (;;)
	{
		rmax = 0.0;
		PL[k] = 0; /* 初期化必須。tRhx[j]が全て0.0の時に問題あり。 */
		for (j = 0; j < XS; ++j)
		{
			if (isExcited[j])
			{
				continue;
			}
			if (rmax < fabs(tRhx[j]))
			{
				PL[k] = j;
				rmax = fabs(tRhx[j]);
			}
		}
		isExcited[*(PL + k)] = 1;
		PA[k] = tRhx[*(PL + k)] / tRhh[0];

		if (k == NoP)
		{
			break;
		}
		for (j = 0; j < XS; ++j)
		{
			pool = abs(j - (PL[k]));
			tRhx[j] -= PA[k] * Rhh[pool];
		}
		++k;
	}
}

/*==============================*/
/*== procedure : PS4          ==*/
/*==============================*/
/*== パルス重複探索法(H15)    ==*/
/*== 佐藤のプログラムを改造   ==*/
/*==============================*/
/*== Rhx : 相互相関関数(入力) ==*/
/*== Rhh : 自己相関関数(入力) ==*/
/*== XS  : 配列長(入力)       ==*/
/*== NoP : パルス数(入力)     ==*/
/*== PL  : パルス位置(出力)   ==*/
/*== PA  : パルス振幅(出力)   ==*/
/*==============================*/
void PS4(double *Rhx, double *Rhh, int XS, int NoP, int *PL, double *PA)
{
	/*-- var --*/
	double Rmax, PAtemp;
	int i, j, bingo, tt, pool, PLtemp, *exist_pulse;

	/*-- cast --*/
	exist_pulse = (int *)calloc(XS, sizeof(int));

	/*-- begin --*/
	bingo = 0;

	/*1番めのパルス位置の探索*/
	Rmax = Rhx[0];
	PL[1] = 0;
	PLtemp = 0;
	for (j = 1; j < XS; j++)
	{
		if (fabs(Rmax) < fabs(Rhx[j]))
		{
			Rmax = Rhx[j];
			PL[1] = j;
			PLtemp = j;
		}
	}
	PA[1] = Rmax / Rhh[0];
	PAtemp = Rmax / Rhh[0];
	exist_pulse[PLtemp] = 1;
	/*== k番めのパルス位置の探索(k=2～PN) ==*/
	for (i = 2; i <= NoP; i++)
	{
		Rmax = 0;
		for (j = 0; j < XS; j++)
		{
			pool = abs(j - PLtemp);
			Rhx[j] = Rhx[j] - PAtemp * Rhh[pool];
		}

		for (j = 0; j < XS; j++)
		{
			if ((fabs(Rmax) < fabs(Rhx[j])) && (exist_pulse[j] <= 10))
			{
				Rmax = Rhx[j];
				PL[i] = j;
				PLtemp = j;
			}
		}

		PAtemp = Rmax / Rhh[0];

		/*== パルス振幅決定 ==*/
		if (exist_pulse[PLtemp] == 0)
		{
			/*== 重複していないとき ==*/
			PA[i] = PAtemp;
		}
		else
		{
			/*== どこを重複探索したか調べる ==*/
			for (tt = 1; tt < i; tt++)
			{
				if (PLtemp == PL[tt])
				{
					bingo = tt;
					break;
				}
			}
			PA[bingo] += PAtemp;
			i--;
		}
		exist_pulse[PLtemp] += 1;
	}
	free(exist_pulse);
}

/*==============================*/
/*== procedure : PS5          ==*/
/*==============================*/
/*==     パルス振幅最適化     ==*/
/*==============================*/
/*== Rhx : 相互相関関数(入力) ==*/
/*== Rhh : 自己相関関数(入力) ==*/
/*== XS  : 配列長(入力)       ==*/
/*== NoP : パルス数(入力)     ==*/
/*== PL  : パルス位置(出力)   ==*/
/*== PA  : パルス振幅(出力)   ==*/
/*==============================*/
void PS5(double *Rhx, double *Rhh, int XS, int NoP, int *PL, double *PA)
{
	/*-- var --*/
	int hoge = 1;
	int *p, i, j, r, k, temp, n;
	double *q, *f, *g, l[XS + 1][NoP + 1], tempd, *tempA;

	/*-- cast --*/
	p = (int *)calloc(XS + 1, sizeof(int));
	f = (double *)calloc(XS + 1, sizeof(double));
	g = (double *)calloc(XS + 1, sizeof(double));
	q = (double *)calloc(XS + 1, sizeof(double));
	tempA = (double *)calloc(XS + 1, sizeof(double));

	/*-- begin --*/

	/*-- 初期化 --*/

	p[0] = 0;
	f[0] = 0;
	g[0] = 0;

	/*-- set --*/
	for (i = 1; i <= XS; i++)
	{
		p[i] = i;
		f[i] = (*(Rhx + i - 1));
		g[i] = (*(Rhh + 0));
	}

	/*-- choose n_1 = i s.t f(i)^2 is max --*/
	tempd = 0.0;
	for (i = 1; i <= XS; i++)
	{
		tempA[i] = (f[i] * f[i]);
	}
	for (i = 1; i <= XS; i += 1)
	{
		if (tempd < tempA[i] && (i % hoge == 0))
		{
			tempd = tempA[i];
			*(PL + 1) = i;
		}
	}

	/*-- interchange --*/
	temp = p[1];
	p[1] = p[*(PL + 1)];
	p[*(PL + 1)] = temp;

	/*-- set --*/
	g[*(PL + 1)] = sqrt(g[*(PL + 1)]);
	q[1] = f[*(PL + 1)] / g[*(PL + 1)];
	j = 1;

	/*-- 6 roop --*/
	/*-- 7,12で使用するl[p[i]][j]の設定 --*/
	do
	{
		r = p[j];
		for (i = j + 1; i <= XS; i++)
		{
			k = p[i];
			tempd = 0.0;
			for (n = 1; n < j; n++)
			{
				tempd = tempd + l[k][n] * l[r][n];
			}
			l[k][j] = (Rhh[abs(k - r)] - tempd) / g[r];
		}

		/*-- 7 compute --*/
		for (i = j + 1; i <= XS; i++)
		{
			k = p[i];
			g[k] = g[k] - l[k][j] * l[k][j];
			f[k] = f[k] - l[k][j] * q[j];
		}

		/*-- 8 choose n_j --*/
		j++;
		tempd = 0.0;
		for (i = j; i <= XS; i++)
		{
			tempA[i] = f[p[i]] * f[p[i]] / g[p[i]];
		}
		for (i = j; i <= XS; i += 1)
		{
			if (tempd < tempA[i] && (i % hoge == 0))
			{
				tempd = tempA[i];
				*(PL + j) = i;
			}
		}

		/*-- 9 interchange --*/
		temp = p[j];
		p[j] = p[*(PL + j)];
		p[*(PL + j)] = temp;

		/*-- 10 set --*/
		g[p[j]] = sqrt(g[p[j]]);
		q[j] = f[p[j]] / g[p[j]];
	} while (j <= NoP);

	/*-- 12 optimize --*/
	j = NoP;
	while (j > 0)
	{
		tempd = 0.0;
		for (k = j + 1; k <= NoP; k++)
		{
			tempd = tempd + l[p[k]][j] * (*(PA + k));
		}
		*(PA + j) = (q[j] - tempd) / g[p[j]];
		*(PL + j) = p[j] - 1;
		j--;
	}
	free(p);
	free(f);
	free(g);
	free(p);
	free(tempA);
}

/*==============================*/
/*== procedure : PS6          ==*/
/*==============================*/
/*== 改良型パルス重複探索法   ==*/
/*== 佐藤のプログラムを改造   ==*/
/*== NoP本パルスを立てた後も修正を行う ==*/
/*==============================*/
/*== Rhx : 相互相関関数(入力) ==*/
/*== Rhh : 自己相関関数(入力) ==*/
/*== XS  : 配列長(入力)       ==*/
/*== NoP : パルス数(入力)     ==*/
/*== PL  : パルス位置(出力)   ==*/
/*== PA  : パルス振幅(出力)   ==*/
/*==============================*/
void PS6(double *Rhx, double *Rhh, int XS, int NoP, int *PL, double *PA)
{
	/*-- var --*/
	double Rmax, PAtemp;
	int i, j, bingo, tt, pool, PLtemp, *exist_pulse;
	int hit;
	int Smax = 10;

	/*-- cast --*/
	exist_pulse = (int *)calloc(XS, sizeof(int));

	/*-- begin --*/
	bingo = 0;

	/*1番めのパルス位置の探索*/
	Rmax = Rhx[0];
	PL[1] = 0;
	PLtemp = 0;
	for (j = 1; j < XS; j++)
	{
		if (fabs(Rmax) < fabs(Rhx[j]))
		{
			Rmax = Rhx[j];
			PL[1] = j;
			PLtemp = j;
		}
	}
	PA[1] = Rmax / Rhh[0];
	PAtemp = Rmax / Rhh[0];
	exist_pulse[PLtemp] = 1;
	/*== k番めのパルス位置の探索(k=2～PN) ==*/
	for (i = 2; i <= NoP; i++)
	{
		Rmax = 0;
		for (j = 0; j < XS; j++)
		{
			pool = abs(j - PLtemp);
			Rhx[j] = Rhx[j] - PAtemp * Rhh[pool];
		}

		for (j = 0; j < XS; j++)
		{
			if ((fabs(Rmax) < fabs(Rhx[j])) && (exist_pulse[j] <= Smax))
			{
				Rmax = Rhx[j];
				PL[i] = j;
				PLtemp = j;
			}
		}

		PAtemp = Rmax / Rhh[0];

		/*== パルス振幅決定 ==*/
		if (exist_pulse[PLtemp] == 0)
		{
			/*== 重複していないとき ==*/
			PA[i] = PAtemp;
		}
		else
		{
			/*== どこを重複探索したか調べる ==*/
			for (tt = 1; tt < i; tt++)
			{
				if (PLtemp == PL[tt])
				{
					bingo = tt;
					break;
				}
			}
			PA[bingo] += PAtemp;
			i--;
		}
		exist_pulse[PLtemp] += 1;
	}
	hit = 1;
	while ((hit == 1) && (PAtemp * PAtemp > 1))
	{
		hit = 0;
		Rmax = 0;
		for (j = 1; j <= NoP; j++)
		{
			pool = abs(PL[j] - PLtemp);
			Rhx[PL[j]] = Rhx[PL[j]] - PAtemp * Rhh[pool];
		}

		for (j = 1; j <= NoP; j++)
		{
			if ((fabs(Rmax) < fabs(Rhx[PL[j]])) &&
				(exist_pulse[PL[j]] < Smax))
			{
				Rmax = Rhx[PL[j]];
				hit = 1;
				PLtemp = PL[j];
			}
		}

		PAtemp = Rmax / Rhh[0];
		/*== どこを重複探索したか調べる ==*/
		if (hit == 1)
		{
			for (tt = 1; tt <= NoP; tt++)
			{
				if (PLtemp == PL[tt])
				{
					bingo = tt;
					break;
				}
			}
		}
		//printf("Pulse Amplitude[%d] %f->%f\n",bingo,PA[bingo],PA[bingo]+PAtemp);
		PA[bingo] += PAtemp;
		exist_pulse[PLtemp]++;
	}
	free(exist_pulse);
}
/*==============================*/
/*== procedure : PS6_3          ==*/
/*==============================*/
/*== 改良型パルス重複探索法   ==*/
/*== 佐藤のプログラムを改造   ==*/
/*== NoP本パルスを立てた後も修正を行う ==*/
/*==============================*/
/*== Rhx : 相互相関関数(入力) ==*/
/*== Rhh : 自己相関関数(入力) ==*/
/*== XS  : 配列長(入力)       ==*/
/*== NoP : パルス数(入力)     ==*/
/*== PL  : パルス位置(出力)   ==*/
/*== PA  : パルス振幅(出力)   ==*/
/*==============================*/
void PS6_3(double *Rhx, double *Rhh, int XS, int NoP, int *PL, double *PA)
{
	/*-- var --*/
	double Rmax, PAtemp;
	int i, j, bingo, tt, pool, PLtemp, *exist_pulse;

	/*-- cast --*/
	exist_pulse = (int *)calloc(XS, sizeof(int));

	/*-- begin --*/
	bingo = 0;

	/*1番めのパルス位置の探索*/
	Rmax = Rhx[0];
	PL[1] = 0;
	PLtemp = 0;
	for (j = 1; j < XS; j++)
	{
		if (fabs(Rmax) < fabs(Rhx[j]))
		{
			Rmax = Rhx[j];
			PL[1] = j;
			PLtemp = j;
		}
	}
	PA[1] = Rmax / Rhh[0];
	PAtemp = Rmax / Rhh[0];
	exist_pulse[PLtemp] = 1;

	/*== k番めのパルス位置の探索(k=2～PN) ==*/
	for (i = 2; i <= NoP; i++)
	{
		Rmax = 0;

		for (j = 0; j < XS; j++)
		{ /*-- Rhxの更新 --*/
			pool = abs(j - PLtemp);
			//if(pool <= 4)
			Rhx[j] = Rhx[j] - PAtemp * Rhh[pool];
		}

		for (j = 0; j < XS; j++)
		{
			if ((fabs(Rmax) < fabs(Rhx[j])))
			{
				Rmax = Rhx[j];
				PL[i] = j;
				PLtemp = j;
			}
		}

		PAtemp = Rmax / Rhh[0];

		/*-- パルス振幅決定 --*/
		if (exist_pulse[PLtemp] == 0)
		{ /*-- 重複していないとき --*/
			PA[i] = PAtemp;
		}
		else
		{ /*-- どこを重複探索したか調べる --*/
			for (tt = 1; tt < i; tt++)
			{
				if (PLtemp == PL[tt])
				{
					bingo = tt;
					break;
				}
			}
			PA[bingo] += PAtemp;
			i--;
		}
		exist_pulse[PLtemp] = 1;
	}

	while ((PAtemp * PAtemp > 1))
	{ /*-- 重複探索を続ける --*/
		Rmax = 0;
		for (j = 1; j <= NoP; j++)
		{ /*-- Rhxの更新 --*/
			pool = abs(PL[j] - PLtemp);
			Rhx[PL[j]] = Rhx[PL[j]] - PAtemp * Rhh[pool];
		}

		for (j = 1; j <= NoP; j++)
		{ /*-- パルス振幅の計算 --*/
			if ((fabs(Rmax) < fabs(Rhx[PL[j]])))
			{
				Rmax = Rhx[PL[j]];
				PLtemp = PL[j];
				bingo = j;
			}
		}
		/*-- パルス振幅決定 --*/
		PAtemp = Rmax / Rhh[0];
		PA[bingo] += PAtemp;
	}
	free(exist_pulse);
}

//2次元
//ブロックサイズは16*16で固定
void PS6_3_2D(double Rhx[16][16], double Rhh[16][16], int NoP,
			  int PL_X[256], int PL_Y[256], double PA[256])
{
	/*-- var --*/
	double Rmax, PAtemp;
	int i, j, tt, p;
	int bingo, pool_X, pool_Y;
	int PLtemp_X, PLtemp_Y;
	int exist_pulse[16][16];

	/*-- clear --*/
	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 16; j++)
		{
			exist_pulse[i][j] = 0;
		}
	}
	/*-- begin --*/
	bingo = 0;

	/*1番めのパルス位置の探索*/
	Rmax = Rhx[0][0];
	PL_X[1] = PL_Y[1] = 0;
	PLtemp_X = PLtemp_Y = 0;
	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 16; j++)
		{
			if (fabs(Rmax) < fabs(Rhx[i][j]))
			{
				Rmax = Rhx[i][j];
				PL_Y[1] = i;
				PL_X[1] = j;
				PLtemp_Y = i;
				PLtemp_X = j;
			}
		}
	}
	PA[1] = Rmax / Rhh[0][0];
	PAtemp = Rmax / Rhh[0][0];
	exist_pulse[PLtemp_Y][PLtemp_X] = 1;
	//printf("# PA[1] = %f:(%d,%d)\n",PAtemp,PLtemp_X,PLtemp_Y);
	/*== k番めのパルス位置の探索(k=2～PN) ==*/
	for (p = 2; p <= NoP; p++)
	{
		Rmax = 0;
		for (i = 0; i < 16; i++)
		{
			for (j = 0; j < 16; j++)
			{
				/*-- Rhxの更新 --*/
				pool_Y = abs(i - PLtemp_Y);
				pool_X = abs(j - PLtemp_X);
				Rhx[i][j] = Rhx[i][j] - PAtemp * Rhh[pool_Y][pool_X];
			}
		}

		/*-- Rmaxの探索 --*/
		for (i = 0; i < 16; i++)
		{
			for (j = 0; j < 16; j++)
			{
				if (fabs(Rmax) < fabs(Rhx[i][j]))
				{
					Rmax = Rhx[i][j];
					PL_Y[p] = i;
					PL_X[p] = j;
					PLtemp_Y = i;
					PLtemp_X = j;
				}
			}
		}
		//printf("#PL[%d] = [%d][%d]\n",p,PLtemp_Y,PLtemp_X);
		PAtemp = Rmax / Rhh[0][0];

		/*-- パルス振幅決定 --*/
		if (exist_pulse[PLtemp_Y][PLtemp_X] == 0)
		{ /*-- 重複していないとき --*/
			PA[p] = PAtemp;
			//printf("# PA[%d] = %f:(%d,%d)\n",p,PAtemp,PLtemp_X,PLtemp_Y);
		}
		else
		{ /*-- どこを重複探索したか調べる --*/
			for (tt = 1; tt < p; tt++)
			{
				if ((PLtemp_Y == PL_Y[tt]) && (PLtemp_X == PL_X[tt]))
				{
					bingo = tt;
					break;
				}
			}
			PA[bingo] += PAtemp;
			p--;
		}
		exist_pulse[PLtemp_Y][PLtemp_X] = 1;
	}

	int tX, tY;
	while ((PAtemp * PAtemp > 1))
	{ /*-- 重複探索を続ける --*/
		Rmax = 0;
		for (j = 1; j <= NoP; j++)
		{ /*-- Rhxの更新 --*/
			tX = PL_X[j];
			tY = PL_Y[j];
			pool_Y = abs(tY - PLtemp_Y);
			pool_X = abs(tX - PLtemp_X);
			Rhx[tY][tX] = Rhx[tY][tX] - PAtemp * Rhh[pool_Y][pool_X];
		}

		for (j = 1; j <= NoP; j++)
		{ /*-- パルス振幅の計算 --*/
			if (fabs(Rmax) < fabs(Rhx[PL_Y[j]][PL_X[j]]))
			{
				Rmax = Rhx[PL_Y[j]][PL_X[j]];
				PLtemp_Y = PL_Y[j];
				PLtemp_X = PL_X[j];
				bingo = j;
			}
		}
		/*-- パルス振幅決定 --*/
		PAtemp = Rmax / Rhh[0][0];
		PA[bingo] += PAtemp;
	}
}
