/*==============================*/
/*==     bmp2pgm (color)      ==*/
/*==     bmp -> pgm (P6)      ==*/
/*==  last update 2008/06/19  ==*/
/*==  by Takashi Kashiwagi    ==*/
/*==============================*/

#include <stdio.h>

unsigned char l_rd_int(double A)
{
  int temp;
  temp = (A+0.5);
  if(temp<0)
    {
      return 0;
    }
  else if(temp>255)
    {
      return 255;
    }
  else
    {
      return temp;
    }
}

typedef struct Picture
{
  int x;             /* x方向のピクセル数 */
  int y;             /* y方向のピクセル数 */
  unsigned char *r;  /* R要素の輝度(0〜255) */
  unsigned char *g;  /* G要素の輝度(0〜255) */
  unsigned char *b;  /* B要素の輝度(0〜255) */
  unsigned char *a;  /* α値(透明度)(0〜255) */
  /* ピクセルのデータは左上から右下へ横方向に操作した順で格納 */
}Picture;

/* 構造体メモリ確保＆初期化関数 */
Picture* newPicture()
{
  /*-- var --*/
  Picture *pPic;
  pPic = (Picture*)malloc(sizeof(Picture));

  /*-- begin --*/
  if(pPic == NULL){
    return NULL;                  /* メモリ確保できない */
  }
  pPic->x = 0;
  pPic->y = 0;
  pPic->r = NULL;
  pPic->g = NULL;
  pPic->b = NULL;
  pPic->a = NULL;
  return pPic;
}



/* 構造体メモリ解放関数 */
void deletePicture(Picture *pPic)
{
  free(pPic->r);
  free(pPic->g);
  free(pPic->b);
  free(pPic->a);
  free(pPic);
  pPic = NULL;
}


/*=======================*/
/*== RGB -> YCbCr      ==*/
/*== Pitureのポインタを返す ==*/
/*== ただしCbCrはバイアスしてある ==*/
/*=======================*/
Picture* Trans_YCbCr(Picture *inPic)
{
  /*-- var --*/
  int i,j,XS,YS;
  double tR,tG,tB;
  Picture *pPic;
  unsigned char *pR,*pG,*pB,*pY,*pCb,*pCr;
  
  /*-- begin --*/

  pPic = newPicture();

  XS = inPic->x;
  YS = inPic->y;
  
  pPic->x = XS;
  pPic->y = YS;
  pR = inPic->r;
  pG = inPic->g;
  pB = inPic->b;
  
  pPic->r = pY = (unsigned char*)calloc(XS*YS,sizeof(unsigned char));
  pPic->g = pCb = (unsigned char*)calloc(XS*YS,sizeof(unsigned char));
  pPic->b = pCr = (unsigned char*)calloc(XS*YS,sizeof(unsigned char));
  
  for(i=0;i<YS;i++)
    {
      for(j=0;j<XS;j++)
	{
	  tR = *(pR+XS*i+j);
	  tG = *(pG+XS*i+j);
	  tB = *(pB+XS*i+j);
	  *(pY+XS*i+j) = l_rd_int(0.299*tR + 0.587*tG + 0.114*tB);
	  *(pCb+XS*i+j) =l_rd_int(128.0 - 0.1687*tR - 0.3313*tG + 0.500*tB);
	  *(pCr+XS*i+j) = l_rd_int(128.0 + 0.500*tR - 0.4187*tG - 0.0813*tB);
	}
    }
 
  return pPic;
}


/*=======================*/
/*== YCbCr -> RGB      ==*/
/*== Pitureのポインタを返す ==*/
/*== ただしCbCrはバイアスしてある ==*/
/*=======================*/
Picture* Trans_RGB(Picture *inPic)
{
  /*-- var --*/
  int i,j,XS,YS;
  double tY,tCb,tCr;
  Picture *pPic;
  unsigned char *pR,*pG,*pB,*pY,*pCb,*pCr;
  
  /*-- begin --*/
  pPic = newPicture();

  XS = inPic->x;
  YS = inPic->y;
  
  pPic->x = XS;
  pPic->y = YS;
  pY = inPic->r;
  pCb = inPic->g;
  pCr = inPic->b;
  
  pPic->r = pR = (unsigned char*)calloc(XS*YS,sizeof(unsigned char));
  pPic->g = pG = (unsigned char*)calloc(XS*YS,sizeof(unsigned char));
  pPic->b = pB = (unsigned char*)calloc(XS*YS,sizeof(unsigned char));
  
  for(i=0;i<YS;i++)
    {
      for(j=0;j<XS;j++)
	{
	  tY = (double)*(pY+XS*i+j);
	  tCb = (double)*(pCb+XS*i+j) - 128.0;
	  tCr = (double)*(pCr+XS*i+j) - 128.0;
	  *(pR+XS*i+j) = l_rd_int(tY + 1.402*tCr);
	  *(pG+XS*i+j) = l_rd_int(tY - 0.34414*tCb - 0.71414*tCr);
	  *(pB+XS*i+j) = l_rd_int(tY + 1.772*tCb );
	}
    }
  
  
  return pPic;
}

int* extractionR(Picture *inPic)
{
  /*--  var --*/
  int i,j;
  int XS = inPic->x;
  int YS = inPic->x;
  int *RP;

  /*-- begin --*/
  if((RP = (int*)malloc(XS*YS*sizeof(int))) == NULL)
    {
      printf("memory alloc error : extractionR\n");
      return NULL;
    }

  for(i=0;i<YS;i++)
    {//Ucharからintへコピー
      for(j=0;j<XS;j++)
	{	  
	  RP[XS*i+j] =  *(inPic->r + XS*i+j);
	}
    }		      		     
  return RP;
} 

unsigned char* int2uchar(int *inIMG,int XS,int YS)
{
  /*-- var --*/
  int i,j,temp;
  unsigned char *UcharP;

  /*-- begin --*/
  if((UcharP = (unsigned char*)malloc(XS*YS*sizeof(unsigned char))) == NULL)
    {
      return NULL;
    }

  for(i=0;i<YS;i++)
    {
      for(j=0;j<XS;j++)
	{
	  temp = *(inIMG + XS*i+j);
	  if(temp<0) temp = 0;
	  if(temp > 255) temp = 255;
	  *(UcharP +XS*i+j) =  temp;
	}
    }		      
		     
  return UcharP;
}

double SNR_COLOR(Picture *xdata,Picture *ydata,int XS,int YS)
{
  /*-- var --*/
  int i,j;
  double temp,temp2,temp3,sq,sqR,sqG,sqB;
  unsigned char *xR,*xG,*xB,*yR,*yG,*yB;
  
  /*-- begin --*/
  xR = xdata->r;
  xG = xdata->g;
  xB = xdata->b;
  yR = ydata->r;
  yG = ydata->g;
  yB = ydata->b;

  temp = 0;
  for(i=0;i<YS;i++)
    {    
      for(j=0;j<XS;j++)
	{
	  sqR = *(xR + XS*i + j) - *(yR + XS*i + j);
	  sqG = *(xG + XS*i + j) - *(yG + XS*i + j);
	  sqB = *(xB + XS*i + j) - *(yB + XS*i + j);
	  sq = sqR*sqR +sqG*sqG +sqB*sqB;
	  temp = temp + sq;
	}
    }
  temp2 = (XS*YS*3);
  temp3 = 10*(log10(temp2) + 2*log10(255) - log10(temp));
  return temp3;
}


