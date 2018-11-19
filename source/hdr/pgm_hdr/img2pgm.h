/*======================*/
/*== PGM input&output ==*/
/*== 2008/5/14        ==*/
/*== last update 2008/06/19  ==*/
/*== by Takashi Kashiwagi    ==*/
/*======================*/

#include "/home/seine/kashi/program2/src/pic_hdr/picture.h"
#define MIN_GRAY 0	/* 最小の輝度値 */
#define MAX_GRAY 255	/* 最大の輝度値 */

/*================*/
/*== Yrev       ==*/
/*== 輝度値反転   ==*/
/*================*/
void Yrev(int *img,int XS,int YS)
{
  int i,j;
  for(i=0;i<YS;i++)
    {
      for(j=0;j<XS;j++)
	{
	  *(img + XS*i + j) = MAX_GRAY - *(img + XS*i + j);
	}
    }
}


/*===================*/
/*== SavePgmVS_Pic ==*/
/*==  P5とP6に対応   ==*/
/*===================*/
int SavePgmVS_Pic(char *filename,Picture *InPic,int type)
{
  /*-- var  --*/
  FILE  *out_fp;
  int i,j,XS,YS;
  unsigned char *pR,*pG,*pB,*buf;
  
  /*-- begin --*/
  /*-- 変数チェック --*/
  if(InPic == NULL || (type < 5 || type >6))   
    {
      printf("hoge\n");
      return NULL;
    }
  /*-- ファイルオープン --*/
  if (( out_fp = fopen( filename, "w" )) == NULL ) 
    {
      printf( "Can not open file: %s\n", filename );
      free(buf);
      return(1);
    }
  
  XS = InPic->x;
  YS = InPic->y;
  pR = InPic->r;
  pG = InPic->g;
  pB = InPic->b;
  
  /*== ヘッダー設定 ==*/
  if( fprintf(out_fp,"P%d\n%d %d\n255\n",type,XS,YS) < 0 )
    {     
      return(1);
    }
  
  /* 画像をファイルへ出力する */
  switch(type)
    {
    case 5:
      /*-- cast --*/
      buf = (unsigned char*)calloc(XS,sizeof(unsigned char));
      
      for (i=0;i<YS;i++ ) 
	{
	  for (j=0;j<XS;j++ ) 
	    {
	      buf[j] = *(pR+i*XS+j);
	    }
	  if ( fwrite(buf,1,XS,out_fp) != XS ) 
	    {	/* 1行分書き込めたか確認 */
	      printf( "file write error: %s\n", filename );
	      fclose( out_fp );
	      free(buf);
	      return 1;
	    }
	  
	}
      free(buf);
      break;
    case 6:
      /*-- きっといつか輝度値のチェックも入れる --*/
      buf = (unsigned char*)calloc(3,sizeof(unsigned char));
      for(i=0;i<YS;i++)
	{
	  for(j=0;j<XS;j++)
	    {
	      buf[0] = *(pR+i*XS+j);
	      buf[1] = *(pG+i*XS+j);
	      buf[2] = *(pB+i*XS+j);
	      if (fwrite(buf,1,3,out_fp) != 3 ) 
		{	/* 1画素分書き込めたか確認 */
		  printf( "file write error: %s\n", filename );
		  fclose( out_fp );
		  free(buf);
		  return 1;
		}
	    }
	}
      free(buf);
      break;
    }
  fclose( out_fp );
  
  return(0);
}





/*===================*/
/*== LoadPgmVS_pic ==*/
/*==  P5とP6に対応   ==*/
/*===================*/
Picture* LoadPgmVS_Pic(char *filename)
{
  /*-- var  --*/
  FILE  *in_fp;
  int i,j,type,max,tXS,tYS;
  char tt[256];
  unsigned char *r,*b,*g,*buf;
  Picture *nPic;

  /*-- begin --*/
  nPic = newPicture();

  if (( in_fp = fopen( filename, "r" )) == NULL ) 
    {
      printf( "Can not open file: %s\n", filename );
      return NULL;
    }

  /*-- hdr read --*/
  
  fgets(tt,256,in_fp);
  if(tt[0] != 'P')
    {
      printf("%s isn't PGM files! \n",&tt[0]);
      return NULL;
    }
  sscanf(tt,"P%d",&type);
  
  if((type < 5) || (type > 6))
    {
      printf("Now,this program can't read P%d\n",type);
      return NULL;
    }
  /*-- コメント読み飛ばし --*/
  do fgets(tt,256,in_fp); while (tt[0] == '#');
  sscanf(tt,"%d %d",&tXS,&tYS);
  
  /*-- コメント読み飛ばし --*/
  do fgets(tt,256,in_fp); while (tt[0] == '#');
  sscanf(tt,"%d",&max);
  

  /*-- Picureのメモリ領域を確保 --*/
  nPic->r  = (unsigned char*)calloc(tXS*tYS,sizeof(unsigned char));
  nPic->g  = (unsigned char*)calloc(tXS*tYS,sizeof(unsigned char));
  nPic->b  = (unsigned char*)calloc(tXS*tYS,sizeof(unsigned char));
  
  r = nPic->r;
  g = nPic->g;
  b = nPic->b;
  
  if((r == NULL) || (b == NULL) || (g == NULL))
    {
      deletePicture(nPic);
      fclose( in_fp );
      return NULL;                  /* メモリ確保できない */
    }

  switch(type)
  {
  case 5:
    /*-- cast --*/
    buf = (unsigned char*)calloc(tXS,sizeof(unsigned char));
    
    for (i=0;i<tYS;i++) 
      {
	if( fread(buf,1,tXS,in_fp) != tXS) 
	  {	
	    printf( "file read error: %s\n", filename );
	    fclose( in_fp );
	    deletePicture(nPic);
	    free(buf);
	    return NULL;
	  }
	for (j=0;j<tXS;j++)
	  {
	    *(r+i*tXS+j) = buf[j]; 
	    *(g+i*tXS+j) = buf[j]; 
	    *(b+i*tXS+j) = buf[j]; 
	  }
	//printf("%d\n",*(r+i*tXS+i));
      }
    free(buf);
    break;
  case 6:
    buf = (unsigned char*)calloc(3,sizeof(unsigned char));
    for (j=0;j<tYS;j++) 
      {	
	for (i=0;i<tXS;i++)
	  {
	    if( fread(buf,1,3,in_fp) != 3) 
	      {	
		printf( "file read error: %s\n", filename );
		fclose( in_fp );
		deletePicture(nPic);
		free(buf);
		return NULL;
	      }
	    *(r+j*tXS+i) = buf[0];
	    *(g+j*tXS+i) = buf[1];
	    *(b+j*tXS+i) = buf[2];
	  }
      }
    break;
  }
  nPic->x = tXS;
  nPic->y = tYS;
  fclose( in_fp );
  return nPic;
}
