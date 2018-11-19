//2Dマルチパルス

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "BITOPA1.h"
#include "LP_hdr/LP_analyz.h"
#include "LP_hdr/PulseSearch.h"
#include "LP_hdr/LSP.h"

//各ブロックの分散からパルス数を割り当てる
unsigned char *get_NumP_1D(int *DifIMG, int XS, int YS, int Mean_Num_P, double *ave_P)
{
	/*-- var --*/
	int i, j, tt, Num_P;
	double sum, mean, var, temp, *difhoge;
	unsigned char *NumberOfPulses;

	/*-- begin --*/

	Num_P = YS * Mean_Num_P / 4;
	//printf("# L = %d\n",L);
	NumberOfPulses = (unsigned char *)malloc(YS * sizeof(unsigned char));
	difhoge = (double *)malloc(YS * sizeof(double));
	tt = 0;
	for (i = 0; i < YS; i++)
	{
		sum = 0.0;
		var = 0.0;
		for (j = 0; j < XS; j++)
		{
			sum += *(DifIMG + i * XS + j);
		}

		mean = sum / (double)XS;
		for (j = 0; j < XS; j++)
		{
			temp = (*(DifIMG + i * XS + j) - mean);
			var += sqrt(temp * temp);
		}
		difhoge[tt] = var;
		tt++;
	}

	sum = 0.0;
	for (i = 0; i < YS; i++)
	{
		sum += difhoge[i];
	}
	mean = sum;
	sum = 0;
	for (i = 0; i < YS; i++)
	{
		tt = Num_P * difhoge[i] / mean;
		tt++;
		NumberOfPulses[i] = 4 * tt;
		if (tt >= 64)
			NumberOfPulses[i] = 63 * 4;
		sum += NumberOfPulses[i];
		//printf("# NumberOfPulses[%d]=%d\n",i,NumberOfPulses[i]);
	}
	*ave_P = sum / (double)YS;
	//printf("# The Number Of Pulses = %f\n",*ave_P);
	free(difhoge);
	return NumberOfPulses;
}

/*=====================================*/
/*== procedure : MPC_encode          ==*/
/*=====================================*/
/*== difimgから各パラメータを求める  ==*/
/*=====================================*/
/*== difimg  : 差分画像(入力)        ==*/
/*== ARorder : AR次数(入力)          ==*/
/*== gamma   : 重み付け係数(入力)    ==*/
/*== NoP     : パルス数(入力)        ==*/
/*== NoKP    : KPAの個数(出力)       ==*/
/*== NoPA    : OUTGKの個数(出力)     ==*/
/*== NoPL    : OUTMKの個数(出力)     ==*/
/*== KPA     : PARCOR係数(出力)      ==*/
/*== OutPA   : Pulse Amplitude(出力) ==*/
/*== OutPL   : Pulse Location(出力)  ==*/
/*== XS      : 画像の横サイズ(入力)  ==*/
/*== YS      : 画像の縦サイズ(入力)  ==*/
/*=====================================*/
void MPC_encode_dPA_v2(
	int *difimg, int ARorder, unsigned char *NumberOfPulses,
	int *Num_LSP, int *Num_PA, int *Num_PL,
	double *OutLSP, double *OutPA, unsigned char *OutPL,
	int XS, int YS)
{
	/*-- var --*/
	int i, tt, Num_P;
	int tempNumLSP, tempNumPA, tempNumPL;
	double tempLSP[16];
	int *PL;
	double *PA, *xtemp, *SortPA;
	unsigned char *PLbitA;

	PL = (int *)calloc(XS, sizeof(int));
	PA = (double *)calloc(XS, sizeof(double));
	xtemp = (double *)calloc(XS, sizeof(double));
	SortPA = (double *)calloc(XS, sizeof(double));
	PLbitA = (unsigned char *)calloc(XS, sizeof(unsigned char));

	double *Rxx, *Rhx, *Rhh, *alpha, *hdata, *xwdata, *hwdata;
	double *tAR;
	double gamma;

	Rxx = (double *)calloc(XS, sizeof(double));
	Rhx = (double *)calloc(XS, sizeof(double));
	Rhh = (double *)calloc(XS, sizeof(double));
	alpha = (double *)calloc(XS, sizeof(double));
	hdata = (double *)calloc(XS, sizeof(double));
	xwdata = (double *)calloc(XS, sizeof(double));
	hwdata = (double *)calloc(XS, sizeof(double));
	tAR = (double *)calloc(XS, sizeof(double));
	int L = 0;
	/*-- begin --*/
	tempNumLSP = *Num_LSP;
	tempNumPA = *Num_PA;
	tempNumPL = *Num_PL;

	for (i = 0; i < YS; i++)
	{
		Num_P = NumberOfPulses[L];
		L++;
		gamma = 0.929 * exp(-0.924 * Num_P / (double)XS);
		for (tt = 0; tt < XS; tt++)
		{
			xtemp[tt] = *(difimg + XS * i + tt);
		}

		/*== LPC分析 ==*/
		A_COR_2(xtemp, XS, Rxx);
		LDIM_A2(Rxx, XS, alpha, ARorder);
		LPtoLSP(alpha, &tempLSP[0]);
		LSPtoLP(tAR, &tempLSP[0]);

		/*== インパルス応答の生成 ==*/
		IPR(tAR, ARorder, XS, hdata);

		/*== 重み付けフィルタ ==*/
		WF(xtemp, alpha, xwdata, XS, ARorder, gamma);
		WF(hdata, alpha, hwdata, XS, ARorder, gamma);

		/*== パルス探索 ==*/
		A_COR_2(hwdata, XS, Rhh);
		C_COR_2(hwdata, xwdata, XS, Rhx);
		PS6_3(Rhx, Rhh, XS, Num_P, PL, PA);

		/*== LSPデータ格納 ==*/
		for (tt = 1; tt <= ARorder; tt++)
		{
			*(OutLSP + tempNumLSP) = tempLSP[tt];
			tempNumLSP++;
		}

		/*== パルス振幅をソート ==*/
		SORT_PA_dPA(PL, PA, Num_P, XS, SortPA);

		/*== PLをbitデータに格納 ==*/
		PL_bitA_E(PL, Num_P, XS, PLbitA);

		/*== PAデータ格納 ==*/
		for (tt = 1; tt <= Num_P; tt++)
		{
			*(OutPA + tempNumPA) = SortPA[tt];
			tempNumPA++;
		}

		/*== PLデータ格納 ==*/
		for (tt = 0; tt < XS / 8; tt++)
		{
			*(OutPL + tempNumPL) = PLbitA[tt];
			tempNumPL++;
		}
	}
	*Num_LSP = tempNumLSP;
	*Num_PA = tempNumPA;
	*Num_PL = tempNumPL;

	free(Rxx);
	free(Rhx);
	free(alpha);
	free(hdata);
	free(xwdata);
	free(hwdata);
	free(tAR);

	free(PL);
	free(PA);
	free(xtemp);
	free(SortPA);
	free(PLbitA);
}

/*====================================*/
/*== procedure : MPC_decode         ==*/
/*====================================*/
/*== 各パラメータからMGMimgを求める ==*/
/*====================================*/
/*== MPCimg  : 再生画像(出力)       ==*/
/*== LFimg   : 低周波画像(入力)     ==*/
/*== PLimg   : パルス位置画像(出力) ==*/
/*== ARorder : AR次数(入力)         ==*/
/*== gamma   : 重み付け係数(入力)   ==*/
/*== NoP     : パルス数(入力)       ==*/
/*== InputLSP : Kパラメータ(入力)    ==*/
/*== InputPA : パルス振幅(入力)   ==*/
/*== InputPL : パルス位置(入力)   ==*/
/*== XS      : 画像の横サイズ(入力) ==*/
/*== YS      : 画像の縦サイズ(入力) ==*/
/*====================================*/
void MPC_decode_v2(
	int *MPCimg, int *LFimg, int *PLimg,
	int ARorder, unsigned char *NumberOfPulses,
	double *InputLSP, double *InputPA, unsigned char *InputPL,
	int XS, int YS)
{
	/*-- var --*/
	int i, j;
	int tNoPL, tNoPA, tNoLSP, tt;
	double LSPbuffer[16];
	double *PAbuffer, *Pulses, *DE_buf;
	unsigned char *PLbitbuf;
	int NoP;
	PAbuffer = (double *)calloc(XS, sizeof(double));
	Pulses = (double *)calloc(XS, sizeof(double));
	DE_buf = (double *)calloc(XS, sizeof(double));
	PLbitbuf = (unsigned char *)calloc(XS / 8, sizeof(unsigned char));

	/*-- begin --*/
	tNoLSP = 0;
	tNoPA = 0;
	tNoPL = 0;
	int L = 0;
	for (i = 0; i < YS; i++)
	{
		NoP = NumberOfPulses[L];
		L++;
		/*== パルス列の初期化 ==*/
		for (tt = 0; tt < XS; tt++)
		{
			Pulses[tt] = 0;
		}

		/*== データ取出し ==*/
		for (tt = 0; tt < ARorder; tt++)
		{
			LSPbuffer[tt] = *(InputLSP + tNoLSP);
			tNoLSP++;
		}

		/*== PA,PL取出し ==*/
		for (tt = 0; tt < NoP; tt++)
		{
			PAbuffer[tt] = *(InputPA + tNoPA);
			tNoPA++;
		}
		for (tt = 0; tt < XS / 8; tt++)
		{
			PLbitbuf[tt] = *(InputPL + tNoPL);
			tNoPL++;
		}

		/*== PLbufの作成 ==*/
		PL_bitA_D(PAbuffer, PLbitbuf, NoP, XS, Pulses);
		for (tt = 0; tt < XS; tt++)
		{
			if (Pulses[tt] != 0)
				*(PLimg + XS * i + tt) = 255;
		}

		/*== LSPによる合成 ==*/
		LSPSynthesizer_2(&LSPbuffer[0], ARorder, Pulses, XS, DE_buf);
		for (j = 0; j < XS; j++)
		{
			*(MPCimg + XS * i + j) = DE_buf[j] + *(LFimg + XS * i + j) + 0.5;
		}
	}

	free(PAbuffer);
	free(Pulses);
	free(DE_buf);
	free(PLbitbuf);
}

void MPC_decode_2D(
	int *MPCimg, int *LFimg, int *PLimg,
	int ARorder, int NoP,
	double *InputLSP_H, double *InputLSP_V,
	double *InputPA, unsigned char *InputPL,
	int XS, int YS)
{
	/*-- var --*/
	int i, j, k, l;
	int tNoPL, tNoPA, tNoLSP, tt;
	double PAbuffer[256], Pulses[16][16], DE_buf[16][16];
	unsigned char PLbitbuf[32];

	double tempLSP_H[16], tempLSP_V[16];
	double alpha_V[256], alpha_H[256]; //入力信号のAR係数（縦・横）

	/*-- begin --*/
	tNoLSP = 0;
	tNoPA = 0;
	tNoPL = 0;

	for (i = 0; i < YS; i += 16)
	{
		for (j = 0; j < XS; j += 16)
		{
			/*== パルス列の初期化 ==*/
			for (k = 0; k < 16; k++)
			{
				for (l = 0; l < 16; l++)
				{
					Pulses[k][l] = 0;
				}
			}

			/*== データ取出し LSP ==*/
			for (tt = 0; tt < ARorder; tt++)
			{
				tempLSP_H[1 + tt] = *(InputLSP_H + tNoLSP);
				tempLSP_V[1 + tt] = *(InputLSP_V + tNoLSP);
				tNoLSP++;
			}

			/*-- 2次元LSPが面倒くさいのLPに逆変換 --*/
			LSPtoLP(&alpha_H[0], &tempLSP_H[0]);
			LSPtoLP(&alpha_V[0], &tempLSP_V[0]);
			alpha_H[0] = -1.0;
			alpha_V[0] = -1.0; //ここか
			//printf("# [%d,%d]: %f,%f,%f,%f\n",i,j,alpha_H[1],alpha_H[2],alpha_H[3],alpha_H[4]);
			//printf("# [%d,%d]: %f,%f,%f,%f\n",i,j,alpha_V[1],alpha_V[2],alpha_V[3],alpha_V[4]);
			/*== PA,PL取出し ==*/
			for (tt = 0; tt < NoP; tt++)
			{
				PAbuffer[tt] = *(InputPA + tNoPA);
				//printf("# PA[%d] = %f\n",tNoPA,PAbuffer[tt]);
				tNoPA++;
			}
			for (tt = 0; tt < 32; tt++)
			{
				PLbitbuf[tt] = *(InputPL + tNoPL);
				tNoPL++;
			}

			/*== PLbufの作成 ==*/
			PL_bitA_D_2D(PAbuffer, &PLbitbuf[0], NoP, Pulses); //ok

			for (k = 0; k < 16; k++)
			{
				for (l = 0; l < 16; l++)
				{
					if (Pulses[k][l] != 0)
						*(PLimg + XS * (i + k) + j + l) = 255;
				}
			}
			/*== LPによる合成 ==*/
			LP_Synthesizer_2D_v2(&alpha_H[0], &alpha_V[0], ARorder, Pulses, DE_buf); //?
			for (k = 0; k < 16; k++)
			{
				for (l = 0; l < 16; l++)
				{
					*(MPCimg + XS * (i + k) + j + l) = DE_buf[k][l] + *(LFimg + XS * (i + k) + j + l) + 0.5;
				}
			}
		}
	}
}
