/*==================================================*/
/*== ヘッダを分割して管理することにします 2008/10 ==*/
/*==================================================*/
/*== LSP ==*/
/*=========*/

/*====================================*/
/*== procedure : LPtoLSP           ==*/
/*== AR係数からLSP係数の決定        ==*/
/*== 次数は4で固定                  ==*/
/*====================================*/
/*== IN_LP   : 線形予測係数(入力)   ==*/
/*== OP_LSP   : LSP係数(出力)        ==*/
/*====================================*/
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
		Local_LSP[i] = acos(tempA[i]) / M_PI;
	}
	for (i = 1; i <= 4; i++)
	{
		*(OP_LSP + i) = Local_LSP[i];
	}
}

/*=======================*/
/*== 8bit quantization ==*/
/*==     LSP -> LP     ==*/
/*==    ARorder = 4    ==*/
/*=======================*/
void LSPtoLP_8bitQ(double *OP_LP, double *IN_LSP)
{
	int i;
	unsigned char ULSP[5];
	double tempLSP[5];
	double r[3], s[3];

	for (i = 1; i <= 4; i++)
	{
		ULSP[i] = *(IN_LSP + i) * 255 + 0.5;
		tempLSP[i] = (double)ULSP[i] * M_PI / 255.0;
		tempLSP[i] = cos(tempLSP[i]);
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

/*=======================*/
/*==     LSP -> LP     ==*/
/*==    ARorder = 4    ==*/
/*=======================*/
void LSPtoLP(double *OP_LP, double *IN_LSP)
{
	int i;
	double tempLSP[5];
	double r[3], s[3];

	for (i = 1; i <= 4; i++)
	{
		tempLSP[i] = *(IN_LSP + i) * M_PI; //M_PIをよく忘れるので注意
		tempLSP[i] = cos(tempLSP[i]);
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

/*===============================================*/
/*==  古川さんのがよく分からないので自分で作る ==*/
/*===============================================*/
void LSPSynthesizer_2(double *IN_LSP, int ARorder,
					  double *Pulses, int XS, double *OUTbuffer)
{
	/*-- var --*/
	double c[ARorder];
	int i, j;
	double ro, rc, re;
	double so, se;
	double Reg_Y;	 /* 出力値を保存するレジスタ */
	double *Reg_ODD;  /* 奇数側のレジスタ */
	double *Reg_EVEN; /* 偶数側のレジスタ */

	/*-- cast --*/
	Reg_ODD = (double *)malloc(ARorder * sizeof(double));
	Reg_EVEN = (double *)malloc(ARorder * sizeof(double));

	/*-- begin --*/
	for (i = 0; i < ARorder; i++)
	{
		c[i] = -2 * cos(*(IN_LSP + i) * M_PI);
		Reg_ODD[i] = 0;
		Reg_EVEN[i] = 0;
	}
	//w_1>0じゃないといけない

	Reg_Y = 0.0;
	for (i = 0; i < ARorder; ++i)
	{
		Reg_ODD[i] = Reg_EVEN[i] = 0.0;
	}

	for (j = 0; j < XS; ++j)
	{
		ro = -Reg_Y * 0.5;
		rc = 0.0;
		re = -Reg_Y * 0.5;
		for (i = 0; i < ARorder / 2; ++i)
		{
			so = Reg_ODD[i * 2] + ro * c[i * 2]; //1
			Reg_ODD[i * 2] = ro;				 //更新
			se = Reg_EVEN[i * 2] + re * c[i * 2 + 1];
			Reg_EVEN[i * 2] = re;
			ro += Reg_ODD[i * 2 + 1];
			Reg_ODD[i * 2 + 1] = so;
			re += Reg_EVEN[i * 2 + 1];
			Reg_EVEN[i * 2 + 1] = se;
			rc += so + se;
		}
		Reg_Y = *(Pulses + j) + (ro + rc - re);
		*(OUTbuffer + j) = Reg_Y;
	}

	free(Reg_EVEN);
	free(Reg_ODD);
}
