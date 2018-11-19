/*==================================*/
/*== 2007/10/22$BGpLZ9'G7$K$h$j:n@.(B ==*/
/*==     $BJd4V$K;HMQ$9$k4X?t72(B     ==*/
/*==    last update 2008/08/04    ==*/
/*==================================*/
/*== $B>\$7$$$3$H$O%"%R%k$NK\(B, ==*/
/*== Ken$B$5$s$N(B``Filters for common Resampling Tasks''$B$*$h$S(B,$B$^$k$b(B  ==*/
/*== $B@=:n=j$N(B2001$BG/!)$"$?$j$NF|5-$rFI$s$G$/$@$5$$(B.                ==*/
/*== $B%9%?%C%/%*!<%P!<%U%m!<$,I]$$$N$G$$$A$$$A(Bcalloc$B$GG[Ns$r3NJ]$7$F$^$9(B ==*/
#include <math.h>		
#define Pi 3.14159265358979

/*========================*/
/*== $B6vBP>N$N@^$jJV$7(B ==*/
/*========================*/
int RD_even_VS(int i,int XS)
{
  /*-- var --*/
  int temp;
  
  /*-- begin --*/
  temp = i;
  while((temp<0)||(temp>=XS))
    {
      if(temp<0)
	{
	  temp = -temp - 1;
	}
      else if(temp>=XS)
	{
	  temp = XS - 1 - (temp % XS);
	}
    }
  return temp;
}

/*====================*/
/*== $B4qBP>N$N@^$jJV$7(B ==*/
/*====================*/
int RD_odd_VS(int i,int XS)
{
  /*-- var --*/
  int temp;
  
  /*-- begin --*/
  temp = i;
  while((temp<0)||(temp>=XS))
    {
      if(temp<0)
	{
	  temp = -temp;
	}
      else if(temp>=XS)
	{
	  temp = XS - 2 - (temp % XS);
	}
    }
  return temp;
}


/*================*/
/*== lanczos$B4X?t(B ==*/
/*== sinc(PI*x)*sinc(PI*x/n) ==*/
/*================*/
double lanczos(double X,int exr,int n)
{
  /*-- var --*/
  double i,j,temp;

  /*-- begin --*/
  j = X / (double)exr;
  i = fabs(j);
  if((0 < i) && (i <= n))
    {temp = sin(Pi*i)/(Pi*i)*sin(Pi*i/n)/(Pi*i/n);}
  else if(i == 0)
    {temp = 1;}
  else 
    {temp = 0;}
  return temp;
}


/*==================================*/
/*== $BJd4V=hM}$r9T$&HO0O$KCm0U$9$k(B ==*/
/*==           Lan4_VS_A2            ==*/
/*==================================*/
void Lan4_VS_A2(double *inIMG,double *outIMG,int exr,int XS,int YS)
{
  /*-- var --*/
  int i,j,t,tXS,tYS,t2XS,t2YS,tempX,tempY;
  double *tempIMG;
  double *l4table;
  
  tempIMG = (double*)calloc(XS*YS,sizeof(double));
  l4table= (double*)calloc(4*exr+1,sizeof(double));
  /*-- begin --*/
  l4table[0] = 1;
  for(i=1;i<=4*exr;i++)
    {
      l4table[i] = lanczos((double)i,exr,4);
    }

  /*-- $BNN0h$N@_Dj(B --*/
  tXS = XS/3;
  tYS = YS/3;
  tempX = tXS/16;
  tempY = tYS/16;
  t2XS = 2*tXS+tempX;
  t2YS = 2*tYS+tempX;
  tXS -= tempX;
  tYS -= tempY;

  /*-- $BJd4V(B --*/
  for(i=tYS;i<t2YS;i+=exr)
    {
      for(j=tXS;j<t2XS;j++)
	{
	  *(tempIMG+XS*i+j) =  *(inIMG+XS*i+j);  
	  for(t = 1; t <= 4*exr; t++)
	    { 
	     *(tempIMG+XS*i+j)  += *(inIMG+XS*i+j+t)*(l4table[t]);
	     *(tempIMG+XS*i+j)  += *(inIMG+XS*i+j-t)*(l4table[t]);
	    } 
	}
    }
  
  for(j=tXS;j<t2XS;j++)
    {
      for(i=tYS;i<t2YS;i++)
	{
	  *(outIMG +XS*i+ j) = *(tempIMG+XS*i+j);
	  for(t = 1; t <= 4*exr; t++)
	    { 
	      *(outIMG+XS*i+j) += *(tempIMG+XS*(i+t)+j)*(l4table[t]);
	      *(outIMG+XS*i+j) += *(tempIMG+XS*(i-t)+j)*(l4table[t]);
	    }
	}
    }
  free(tempIMG);
  free(l4table);
}/* Lan4_VS */


/*=============================*/
/*==          expand         ==*/
/*=============================*/
/*== $B%"%C%W%5%s%W%j%s%0(B      ==*/
/*== miniimg[YS/exr][XS/exr] ==*/
/*== EXimg[YS][XS]           ==*/
/*=============================*/
void expandVS(int *miniimg,double *EXimg,int exr,int XS,int YS)
{
  /*-- var --*/
  int i,j;

  /*-- begin --*/
  for(i = 0;i < (YS/exr);i++)
    {
      for(j = 0;j < (XS/exr);j++)
	{	 
	  *(EXimg + exr*i*XS + exr*j) = *(miniimg + i*XS/exr + j);
	}
    }
}/* expandVS */

/*-- expand end. --*/


/*==================*/
/*==  Matrix copy ==*/
/*==================*/
/*-- copy int to double--*/
void CPMi2d_VS(int *x,double *y,int XS,int YS)
{
  /*-- var --*/
  int i,j;
  
  /*-- begin --*/
  for(i = 0;i < YS ;i++)
    {
      for(j = 0;j < XS ;j++)
	{
	  *(y+i*XS+j) = *(x+i*XS+j);
	}
    }
}
/*-- copy --*/

/*-- copy double to int--*/
void CPMd2i_VS(double *x,int *y,int XS,int YS)
{
  /*-- var --*/
  int i,j;
  
  /*-- begin --*/
  for(i = 0;i < YS ;i++)
    {
      for(j = 0;j < XS ;j++)
	{
	  *(y+i*XS+j) = *(x+i*XS+j) + 0.5;
	}
    }
}
/*-- copy --*/

/*================*/
/*== Max select ==*/
/*================*/
double MaxselVS(double *inimg,int XS)
{
  /*-- var --*/
  int i;
  double max;
  double temp = *inimg;

  /*-- begin --*/
  temp = fabs(temp);
  max = temp;
  for(i=0;i<XS;i++)
    {
      temp = *(inimg + i);
      temp = fabs(temp);
      if(temp> max)
	{
	  max = temp;
	}  
    }
  return max;
}

/*==========================*/
/*==     Entropy_Uchar    ==*/
/*== $B%(%s%H%m%T!<$r5a$a$k(B ==*/
/*==     Uchar $B$KBP1~(B     ==*/
/*==========================*/
double Entropy_Uchar(unsigned char *A,int size)
{
  /*== var ==*/
  int i,data[256],tt;
  double px,px_I,dd;
  double temp = 0;
  
  
  /*== begin ==*/
  for(i=0;i<256;i++)
    {
     data[i] = 0;
    } 
  
  for(i=0;i<size;i++)
    {
      tt = *(A+i);   
      data[tt] = data[tt]+1;
    }
  
  for(i=0;i<256;i++)
    {    
      px = data[i]/(double)size;
      dd = -1 + i/(128.0);
    
      if (px !=0)
	{
	  px_I = 1/px;	
	  temp += px*log(px_I)/log(2);
	}
    }
  return temp;
}

/*==========================*/
/*==     Entropy_char     ==*/
/*== $B%(%s%H%m%T!<$r5a$a$k(B ==*/
/*==      char $B$KBP1~(B     ==*/
/*==========================*/
double Entropy_char(char *A,int size)
{
  /*== var ==*/
  int i,data[256],tt;
  double px,temp,px_I,dd;
  temp = 0;
  /*== begin ==*/
  for(i=0;i<256;i++)
    {
     data[i] = 0;
    } 
  
  for(i=0;i<size;i++)
    {
      tt = *(A+i)+127;    
      data[tt] = data[tt]+1;
    }
  
  for(i=0;i<256;i++)
    {   
      px = data[i]/(double)size;
      dd = -1 + i/(128.0);  
      if (px !=0)
	{
	  px_I = 1/px;
	  temp += px*log(px_I)/log(2);
	}
    }
  return temp;
}

/*==========================*/
/*==    hist_char         ==*/
/*== $B%R%9%H%0%i%`=q$-=P$7(B ==*/
/*==      char $B$KBP1~(B     ==*/
/*==========================*/
void hist_char(char *A,int size)
{
  /*== var ==*/
  int i,data[256],tt;
  double px,temp,px_I,dd;
  temp = 0;
  /*== begin ==*/
  for(i=0;i<256;i++)
    {
     data[i] = 0;
    }   
  for(i=0;i<size;i++)
    {
      tt = *(A+i)+127; 
      data[tt] = data[tt]+1;
    } 
  for(i=0;i<256;i++)
    {
     
      px = data[i]/(double)size;
      dd = -127 + i;
      printf("%f  %d\n",dd,data[i]);
      if (px !=0)
	{
	  px_I = 1/px;
	  temp += px*log(px_I)/log(2);
	}
    }
}

/*==========================*/
/*==    hist_Uchar         ==*/
/*== $B%R%9%H%0%i%`=q$-=P$7(B ==*/
/*==      Uchar $B$KBP1~(B     ==*/
/*==========================*/
void hist_Uchar(unsigned char *A,int size)
{
  /*== var ==*/
  int i,data[256],tt;
  double px,temp,px_I,dd;
  temp = 0;
  /*== begin ==*/
  for(i=0;i<256;i++)
    {
     data[i] = 0;
    }   
  for(i=0;i<size;i++)
    {
      tt = *(A+i); 
      data[tt] = data[tt]+1;
    } 
  for(i=0;i<256;i++)
    {
     
      px = data[i]/(double)size;
      dd = i;
      printf("%f  %f\n",(double)dd/255.0,px);
      if (px !=0)
	{
	  px_I = 1/px;
	  temp += px*log(px_I)/log(2);
	}
    }
}


/*==================*/
/*==== round_d =====*/
/*==   double$B7?(B   ==*/
/*==================*/
double round_d(double A,double MAX)
{
  /*-- var --*/
  double temp;

  /*-- begin --*/
  temp = 0;
  if(A > MAX)
    {
      temp = MAX;
    }
  else if(A<0)
    {
      temp = 0;
    }
  else
    {
      temp = A;
    }
  return temp;
}

/*==================*/
/*==== round_i =====*/
/*==================*/
int hogeint(int A,int MAX)
{
  /*-- var --*/
  int temp;

  /*-- begin --*/
  if(A > MAX)
    {
      temp = MAX;
    }
  else if(A<1)
    {
      temp = 1;
    }
  else
    {
      temp = A;
    }
  return temp;
}

/*==================*/
/*==== round_i =====*/
/*==================*/
int round_int(int A,int MAX)
{
  /*-- var --*/
  int temp;

  /*-- begin --*/
  if(A > MAX)
    {
      temp = MAX;
    }
  else if(A<1)
    {
      temp = 0;
    }
  else
    {
      temp = A;
    }
  return temp;
}



/*=====================================*/
/*== minimize (with Lanczos Window)  ==*/
/*== inIMG[YS][XS] : (i)             ==*/
/*== outIMG[YS/exr][XS/exr] : (o)    ==*/
/*== exr : (i)                       ==*/
/*=====================================*/
void minilanc3VS_A1(int *inIMG,int *outIMG,int exr,int XS,int YS)
{
  /*-- var --*/
  int i,j,k;
  int sXS = XS/exr;
  int tempEXR = 3*exr;
  double wsum;
  double temp;
  double *tempIMG;
  double *L3table;

  tempIMG = (double*)calloc(XS*YS,sizeof(double));
  L3table = (double*)calloc(3*exr+1,sizeof(double));

  /*-- begin --*/
  L3table[0] = 1;
  wsum = 1;
  for(i=1;i<=tempEXR;i++)
    {
      L3table[i] = lanczos(i,exr,3);
      wsum += 2*L3table[i];
    }
   for(i=0;i<=tempEXR;i++)
    {
      L3table[i] = L3table[i]/wsum;
    }
   
   for ( i = 0; i < YS; i++ ) 
     {
       //$BB?J,$3$l$G>/$7$OAa$/$J$k$O$:(B
      for ( j = 0; j < tempEXR; j +=exr ) 
	{	
	  temp = *(inIMG + XS*i + j)*L3table[0]; 
	  for ( k = 1; k <=tempEXR; k++ )
	    {
	      temp += L3table[k]*(*(inIMG + XS*i + RD_odd_VS(j+k,XS)));
	      temp += L3table[k]*(*(inIMG + XS*i + RD_odd_VS(j-k,XS)));
	    }
	  *(tempIMG+XS*i+j/exr) = temp;
	}
      for ( j = tempEXR; j < XS - tempEXR; j +=exr ) 
	{	
	  temp = *(inIMG + XS*i + j)*L3table[0]; 
	  for ( k = 1; k <=tempEXR; k++ )
	    {
	      temp += L3table[k]*(*(inIMG + XS*i + j+k));
	      temp += L3table[k]*(*(inIMG + XS*i + j-k));
	    }
	 *(tempIMG+XS*i+j/exr) = temp;
	}
      for ( j = XS - tempEXR; j < XS; j +=exr ) 
	{	
	  temp = *(inIMG + XS*i + j)*L3table[0]; 
	  for ( k = 1; k <=tempEXR; k++ )
	    {
	      temp += L3table[k]*(*(inIMG + XS*i + RD_odd_VS(j+k,XS)));
	      temp += L3table[k]*(*(inIMG + XS*i + RD_odd_VS(j-k,XS)));
	    }
	 *(tempIMG+XS*i+j/exr) = temp;
	}    
     }
 
  for ( i = 0; i < tempEXR; i+=exr ) 
    {
      for ( j = 0; j < sXS; j++ ) 
	{	
	  temp = *(tempIMG+XS*i+j)*L3table[0];
	  for ( k = 1; k <=tempEXR; k++ )
	    {    
	      temp += L3table[k]*( *(tempIMG+XS*RD_odd_VS(i+k,YS)+j));
	      temp += L3table[k]*( *(tempIMG+XS*RD_odd_VS(i-k,YS)+j));
	    }
	  *(outIMG+ sXS*i/exr +j) = round_int(temp + 0.5,255);
	}
    }
  for ( i = tempEXR; i < YS - tempEXR; i+=exr ) 
    {
      for ( j = 0; j < sXS; j++ ) 
	{	
	  temp = *(tempIMG+XS*i+j)*L3table[0];
	  for ( k = 1; k <=tempEXR; k++ )
	    {    
	      temp += L3table[k]*( *(tempIMG+XS*(i+k)+j));
	      temp += L3table[k]*( *(tempIMG+XS*(i-k)+j));
	    }
	  *(outIMG+ sXS*i/exr +j) = round_int(temp + 0.5,255);
	}
    }
  for ( i = YS - tempEXR; i < YS; i+=exr ) 
    {
      for ( j = 0; j < sXS; j++ ) 
	{	
	  temp = *(tempIMG+XS*i+j)*L3table[0];
	  for ( k = 1; k <=tempEXR; k++ )
	    {    
	      temp += L3table[k]*( *(tempIMG+XS*RD_odd_VS(i+k,YS)+j));
	      temp += L3table[k]*( *(tempIMG+XS*RD_odd_VS(i-k,YS)+j));
	    }
	  *(outIMG+ sXS*i/exr +j) = round_int(temp + 0.5,255);
	}
    }
  free(tempIMG);
  free(L3table);
}/* minilanc2 */


/*==================================*/
/*==     $B??$sCf$N%V%m%C%/Cj=P(B     ==*/
/*== EXXS = 3*XS, EXYS = 3*YS ==*/
/*==================================*/
void EXTR_VS(double *eximg,int *outimg,int XS,int YS,int EXXS,int EXYS)
{
  /*-- var --*/
  int i,j;
 
  /*-- begin --*/ 
  for(i=YS;i<EXYS-YS;i++)
    {
      for(j=XS;j<EXXS-XS;j++)
	{
	  *(outimg + XS*(i-YS) + (j-XS)) = *(eximg + EXXS*i + j);
	}
    }
}


/*==================================*/
/*== 9*XS*YS$B$NBP>]<~4|2hA|$r@8@.(B  ==*/
/*== img[sXS][sYS]:(i)            ==*/
/*== outimg[3*sXS][3*sYS]:(o)     ==*/
/*==================================*/
void GenSynIMG(int *img,int *outIMG,int sXS,int sYS)
{
  /*-- var --*/
  int i,j;
  int EXXS=3*sXS;

  /*-- begin --*/ 

  /*-- $B??$sCf%3%T!<(B --*/
  for(i=0;i<sYS;i++)
    {
      for(j=0;j<sXS;j++)
	{  
	  *(outIMG+EXXS*i+j) = *(img+sXS*(sYS-1-i)+(sXS-1-j));
	  *(outIMG+EXXS*i+j+sXS) = *(img+sXS*(sYS-1-i)+j);
	  *(outIMG+EXXS*i+j+2*sXS) = *(img+sXS*(sXS-1-i)+(sXS-1-j));

	  *(outIMG+EXXS*(i+sYS)+j) = *(img+sXS*i+(sXS-1-j));
	  *(outIMG+EXXS*(i+sYS)+j+sXS) = *(img+sXS*i+j);
	  *(outIMG+EXXS*(i+sYS)+j+2*sXS) = *(img+sXS*i+(sXS-1-j));
	  
	  *(outIMG+EXXS*(i+2*sYS)+j) = *(img+sXS*(sYS-1-i)+(sXS-1-j));
	  *(outIMG+EXXS*(i+2*sYS)+j+sXS) = *(img+sXS*(sYS-1-i)+j);
	  *(outIMG+EXXS*(i+2*sYS)+j+2*sXS) = *(img+sXS*(sYS-1-i)+(sXS-1-j));	  
	}
    }
}

/*=== $BBP>]<~4|2hA|$N@8@.(B end ===*/

/*========================*/
/*==     SNR$B$N7W;;(B      ==*/
/*========================*/
double SNR_hoge(int *xdata,int *ydata,int XS,int YS)
{
  /*-- var --*/
  int i,j;
  double temp,temp2,temp3,sq;
  
  /*-- begin --*/
  temp = 0;
  for(i=0;i<YS;i++)
    {    
      for(j=0;j<XS;j++)
	{
	  sq = *(xdata + XS*i + j) - *(ydata + XS*i + j);
	  sq = sq*sq;
	  temp = temp + sq;
	}
    }
  temp2 = (XS*YS);
  temp3 = 10*(log10(temp2) + 2*log10(255) - log10(temp));
  return temp3;
}

/*===============*/
/*==    DIF    ==*/
/*== C = A - B ==*/
/*===============*/
void DIFVS(int *A,int *B,int *C,int XS,int YS)
{
  /*== var ==*/
  int i,j;
  
  /*== begin ==*/
  for(i=0;i<YS;i++)
    {
      for(j=0;j<XS;j++)
	{
	*(C+XS*i+j) = *(A+XS*i+j) - *(B+XS*i+j);
	}
    }
}


/*==================================*/
/*== $B=L>.2hA|$+$iDc<~GH2hA|$r@8@.(B ==*/
/*==================================*/
void GenLowImgVS(int *miniIMG,int *OUTLI,int XS,int YS,int exr)
{
  /*-- var --*/
  int *mini_synIMG;
  double *d_tempIMG,*d_tempIMG2;

  mini_synIMG = (int*)calloc(3*YS/exr*3*XS/exr,sizeof(int));
  d_tempIMG = (double*)calloc(9*XS*YS,sizeof(double));
  d_tempIMG2 = (double*)calloc(9*XS*YS,sizeof(double));

  /*-- begin --*/
  //printf("GLIVS\n");
  /*-- $BBP>]<~4|2hA|@8@.(B --*/
  GenSynIMG(miniIMG,mini_synIMG,XS/exr,YS/exr);

  /*-- $B3HBg=hM}(B --*/
  expandVS(mini_synIMG,d_tempIMG,exr,3*XS,3*YS);

  /*-- $BJd4V=hM}(B --*/
  Lan4_VS_A2(d_tempIMG,d_tempIMG2,exr,3*XS,3*YS);

  /*-- $B??$sCf@Z=P(B --*/
  EXTR_VS(d_tempIMG2,OUTLI,XS,YS,3*XS,3*YS);  
  
  free(mini_synIMG);
  free(d_tempIMG);
  free(d_tempIMG2);
}
