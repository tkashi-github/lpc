/*==================================*/
/*== 2007/10/19$BGpLZ9'G7$K$h$j:n@.(B ==*/
/*==        $B%S%C%HA`:n4X?t(B        ==*/
/*==    last update 2008/9/12    ==*/
/*==================================*/

typedef unsigned char Uchar ;
typedef char packedC;

/*========================*/
/*== $B%S%C%H5M$a9~$_4X?t(B ==*/
/*== PL($B%Q%k%90LCV(B)[PN] ==*/
/*==    Num_P($B%Q%k%9?t(B)    ==*/
/*==    XS($B%i%$%sD9(B)    ==*/
/*== PLbitA($B=PNO(B)[XS/8] ==*/
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
/*== $B%S%C%HFI$_9~$_4X?t(B ==*/
/*== MKbitA($BF~NO(B)[XS/8] ==*/
/*==    PN($B%Q%k%9?t(B)    ==*/
/*==    XS($B%i%$%sD9(B)    ==*/
/*==   MK_A($B=PNO(B)[XS]   ==*/
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
//$B%Q%k%90LCV$O(BOK
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




