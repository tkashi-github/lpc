/*==================================*/
/*== 2007/10/19柏木孝之により作成 ==*/
/*==        ビット操作関数        ==*/
/*==    last update 2008/9/12    ==*/
/*==================================*/

typedef unsigned char Uchar ;
typedef char packedC;

/*========================*/
/*== ビット詰め込み関数 ==*/
/*== PL(パルス位置)[PN] ==*/
/*==    Num_P(パルス数)    ==*/
/*==    XS(ライン長)    ==*/
/*== PLbitA(出力)[XS/8] ==*/
/*========================*/
void PL_bitA_E(int *PL,int Num_P,int XS,Uchar *PLbitA)
{
  /*-- var --*/
  int i,j;
  Uchar hoge,*temp;

  temp = (Uchar*)calloc(XS,sizeof(Uchar));

  /*-- begin --*/
  for(i=0;i<XS;i++)
    {
      temp[i] = 0;
    }
  for(i=1;i<=Num_P;i++)
    {
      temp[*(PL + i)] = 1;
    }
  
  j = 0;
  for(i=0;i<XS;i+=8)
    {
      hoge = 0;
      hoge = (hoge | (temp[i] & 0x01)) << 1;
      hoge = (hoge | (temp[i+1] & 0x01)) << 1;
      hoge = (hoge | (temp[i+2] & 0x01)) << 1;
      hoge = (hoge | (temp[i+3] & 0x01)) << 1;
      hoge = (hoge | (temp[i+4] & 0x01)) << 1;
      hoge = (hoge | (temp[i+5] & 0x01)) << 1;
      hoge = (hoge | (temp[i+6] & 0x01)) << 1;
      hoge = (hoge | (temp[i+7] & 0x01));
     
      *(PLbitA+j) = hoge;
      j++;
    }
  free(temp);
}

//2D
void PL_bitA_E_2D(int PL_X[256],int PL_Y[256],
		  int Num_P,Uchar PLbitA[256])
{
  /*-- var --*/
  int i,j;
  Uchar hoge,temp[256];

  /*-- begin --*/
  for(i=0;i<256;i++){
      temp[i] = 0;
  }
  for(i=1;i<=Num_P;i++){
    temp[16*PL_Y[i]+PL_X[i]] = 1;
  }
  
  j = 0;
  for(i=0;i<256;i+=8)
    {
      hoge = 0;
      hoge = (hoge | (temp[i] & 0x01)) << 1;
      hoge = (hoge | (temp[i+1] & 0x01)) << 1;
      hoge = (hoge | (temp[i+2] & 0x01)) << 1;
      hoge = (hoge | (temp[i+3] & 0x01)) << 1;
      hoge = (hoge | (temp[i+4] & 0x01)) << 1;
      hoge = (hoge | (temp[i+5] & 0x01)) << 1;
      hoge = (hoge | (temp[i+6] & 0x01)) << 1;
      hoge = (hoge | (temp[i+7] & 0x01));
     
      PLbitA[j] = hoge;
      j++;
    }
}



/*========================*/
/*== ビット読み込み関数 ==*/
/*== MKbitA(入力)[XS/8] ==*/
/*==    PN(パルス数)    ==*/
/*==    XS(ライン長)    ==*/
/*==   MK_A(出力)[XS]   ==*/
/*========================*/
void PL_bitA_D(double *PAbuf,Uchar *PLbitA,int PN,int XS,double *PLSbuf)
{
  /*-- var --*/
  int i,j,tt,*temp;
  Uchar hoge;

  temp = (int*)calloc(XS,sizeof(int));
  /*-- begin --*/

  tt=0;
  for(i=0;i<XS/8;i++)
    {
      hoge = *(PLbitA+i);
      for(j=7;j>=0;j--)
	{
	  temp[tt]=(hoge>>j)&0x01;
	  tt++;
	}
    }

  tt=0;
  for(i=0;i<XS;i++)
    {
      *(PLSbuf+i)=0;
      if(temp[i] != 0)
	{
	  *(PLSbuf+i) = *(PAbuf+tt);
	  tt++;
	} 
    }
  free(temp);
}

//2D
//パルス位置はOK
void PL_bitA_D_2D(double PAbuf[256],Uchar *PLbitA,
		  int PN,double PLSbuf[16][16])
{
  /*-- var --*/
  int i,j,tt,temp[16][16],buf[256];
  Uchar hoge;

  /*-- begin --*/

  tt=0;
  for(i=0;i<32;i++){
    hoge = PLbitA[i];
    for(j=7;j>=0;j--){
      buf[tt]=(hoge>>j)&0x01;
      tt++;
    }
  }

  tt=0;
  for(i=0;i<16;i++){
    for(j=0;j<16;j++){
      temp[i][j] = buf[tt];
      tt++;
    }
  }

  tt=0;
  for(i=0;i<16;i++){
    for(j=0;j<16;j++){
      PLSbuf[i][j]=0;
      if(temp[i][j] != 0){
	PLSbuf[i][j] = PAbuf[tt];
	//printf("# [%d,%d] = %f\n",i,j,PAbuf[tt]);
	tt++;
      } 
    }
  }
}




