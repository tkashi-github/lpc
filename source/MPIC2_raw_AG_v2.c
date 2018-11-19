/*============================
== 1D�Τޤ�
== 2009/
== 
============================*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>	
#include <string.h>
#include <time.h>
#include "hdr/MPIC2_raw_AG.h"
#include "hdr/pgm_hdr/pgmIO.h"
#include "hdr/IP2A4.h"



int main(int argc, char *argv[])
{
  /*-- var --*/
  FILE *tempFP,*tempFP2;
  char *OP_PAdat,*OP_LSPdat;
  
  /*-- �ե�����͡��� --*/
  char *IN_filename,*OP_mpic;

  /*-- ���������� --*/
  int XS,YS;//������������
  
  /*-- MPCʬ�ϤǻȤ��ѥ�᡼�� --*/
  int ARorder; //AR����
  int exR; //�̾���������Ψ

  /*-- �����Ѻ���ΰ� --*/
  int *OriginIMG;//������
  int *tempIMG;//��������
  int *RIMG,RI_SIZE;//�̾�����
  int *lowIMG,*PLIMG;//����Ȳ������ѥ륹���ֲ���
  int *dec_mpcIMG;//��������
 
  /*-- Picture��¤�δط� --*/
  Picture *OriginPic,*OutPic;//Picture��¤��
  int type; //pgm�Υ�����Ƚ��
 
  /*-- LSP PA PL�Υ����� --*/
  int Num_LSP,Num_PA,Num_PL,Num_P;
    
  /*-- �Ƽ�ǡ��� --*/
  unsigned char *temp_PL,*NumberOfPulses;
  double *temp_PA,*temp_LSP;
  double ave_P;

  /*-- begin --*/
  /* ���ޥ�ɥ饤������Υ����å� */
  if ( argc != 7 ){
    printf("# ./a.out <exr> <pulses> <IN.pgm> <OUT.pgm> <*.PAdat> <*.LSPdat>\n");
    exit( 1 );
  }

  /*-- Load Expansion Rate --*/
  exR = atoi(argv[1]);
  
  /*-- Load gamma value --*/
  
  /*-- Load  Number_of_Pulses --*/
  Num_P = atoi(argv[2]);

  /*-- Load ARorder value --*/
  ARorder = 4; //���Ǹ���ˤ���.

  /*-- new(Picture) --*/
  OriginPic = newPicture();

  /*-- Read .pgm file --*/
  IN_filename = argv[3];
  if((OriginPic = LoadPgmVS_Pic(IN_filename,&type)) == NULL)
    {// �ե����뤬�ʤ�
      printf("# not found : %s\n",IN_filename);
      goto dispose_0;
    }
  if(type != 5)
    {// ��Υ�������Ǥʤ�
      printf("# This file isn't P5 : %s\n",IN_filename);
      goto dispose_1;
    }
  
  OP_PAdat = (char*)malloc(30*sizeof(char));
  strcpy(OP_PAdat,argv[5]);
  printf("# argv[5] = %s\n",OP_PAdat);
  OP_LSPdat = (char*)malloc(30*sizeof(char));
  strcpy(OP_LSPdat,argv[6]);
  printf("# argv[6] = %s\n",OP_LSPdat);
  
  /*-- �����������ɤ߼�� --*/
  XS = OriginPic->x;
  YS = OriginPic->y;
  //printf("# Image Size = %d x %d\n",XS,YS);
  
  /*-- ������� --*/
  OriginIMG = extractionR(OriginPic);//������
  RI_SIZE = XS*YS/(exR*exR);//�̾������Υ�����
  RIMG = (int*)malloc(RI_SIZE*sizeof(int));//�̾�����
  lowIMG = (int*)malloc(XS*YS*sizeof(int)); //����Ȳ���
  PLIMG = (int*)malloc(XS*YS*sizeof(int));//�ѥ륹���ֲ���
  tempIMG = (int*)malloc(XS*YS*sizeof(int));//
  temp_LSP = (double*)malloc(XS*YS/16*sizeof(double)); 
  temp_PA = (double*)malloc(XS*YS*sizeof(double));
  temp_PL = (unsigned char*)malloc(XS*YS*sizeof(unsigned char));
  dec_mpcIMG = (int*)malloc(XS*YS*sizeof(int));

  /*-- �̾��������� --*/
  minilanc3VS_A1(OriginIMG,RIMG,exR,XS,YS);

  /*-- �̾�������������Ȳ��������� --*/
  GenLowImgVS(RIMG,lowIMG,XS,YS,exR);

  /*-- ��������ѿ��ν���� --*/
  Num_LSP = 0;
  Num_PA = 0;
  Num_PL = 0;

  /*== ��ʬ�������� ==*/
  DIFVS(OriginIMG,lowIMG,tempIMG,XS,YS);
  NumberOfPulses = get_NumP_1D(tempIMG,XS,YS,Num_P,&ave_P);
 
  /*== MPC encode ==*/
  MPC_encode_dPA_v2(
	     tempIMG,ARorder,NumberOfPulses,&Num_LSP,&Num_PA,&Num_PL,
	     &temp_LSP[0],&temp_PA[0],&temp_PL[0],XS,YS
	     );
  /*-- ��������å� --*/ 
  printf("# Ex rate = 1/%d \n",exR);
  printf("# The number of LSP = %d \n",Num_LSP);
  printf("# The number of PA = %d \n",Num_PA);
  printf("# The number of PL = %d \n",Num_PL);
 

 
  /*-- MPC�ǥ����� --*/
  MPC_decode_v2(
	     dec_mpcIMG,lowIMG,PLIMG,
	     ARorder,NumberOfPulses,
	     temp_LSP,temp_PA,temp_PL,
	     XS,YS
	     );
 
  /*-- �����Ѥ�picture��¤�Τν��� --*/
  OutPic = newPicture();
  OutPic->r = int2uchar(&dec_mpcIMG[0],XS,YS);
  OutPic->g = int2uchar(&dec_mpcIMG[0],XS,YS);
  OutPic->b = int2uchar(&dec_mpcIMG[0],XS,YS);
  OutPic->x = XS;
  OutPic->y = YS;

  /*-- �ƹ���������¸ --*/
  OP_mpic = argv[4];  
  if(SavePgmVS_Pic(OP_mpic,OutPic,5) != 0){
    printf("# SavePgmVS_Pic error : %s\n",OP_mpic);
    goto dispose_3;
  }
  
  char *tempname;

  tempname = OP_mpic; 
  strcat(tempname,"p");
  deletePicture(OutPic);
  OutPic = newPicture();
  OutPic->r = int2uchar(&PLIMG[0],XS,YS);
  OutPic->g = int2uchar(&PLIMG[0],XS,YS);
  OutPic->b = int2uchar(&PLIMG[0],XS,YS);
  OutPic->x = XS;
  OutPic->y = YS;
  if(SavePgmVS_Pic(tempname,OutPic,5) != 0){
    printf("SavePgmVS_Pic error : %s\n",tempname);
    return 1;
  }
  
  /*-- �����ɥ֥å������Τ���PA���̤���¸ --*/
  if((tempFP = fopen(OP_PAdat,"wb")) == NULL){
    printf( "# Can not open file: %s\n", "PA_data");
    goto dispose_3;
  }
  if(fwrite(&Num_PA,4,1,tempFP) != 1){
    printf( "# file write error: %s\n", "PA_data");
    fclose( tempFP );
    goto dispose_3;
  }
  if ( fwrite(temp_PA, 8, Num_PA,tempFP) !=  Num_PA) {	
    printf( "# outputPA file write error: %s\n","PA_data");
    fclose( tempFP );
    goto dispose_3;
  }
  fclose( tempFP );
  
  if((tempFP2 = fopen(OP_LSPdat,"wb")) == NULL ) {
    printf( "# Can not open file: %s\n", "LSP_data");
    goto dispose_3;
  }
  if(fwrite(&Num_LSP,4,1,tempFP2) != 1){
    printf( "# file write error: %s\n", "LSP_data");
    fclose( tempFP2 );
    goto dispose_3;
  }
  if ( fwrite(temp_LSP, 8, Num_LSP,tempFP2) !=  Num_LSP) {	
    printf( "# outputLSP file write error: %s\n","LSP_data");
    fclose( tempFP2 );
    goto dispose_3;
  }
  fclose( tempFP2 );
  
  /*-- SNR���� --*/
  //printf("# Pulses = %d, PSNR = %2.4f[dB]\n",Num_P,SNR_hoge(OriginIMG,dec_mpcIMG,XS,YS));
  printf("%f %2.4f\n",ave_P/(double)XS,SNR_hoge(OriginIMG,dec_mpcIMG,XS,YS));
  
 dispose_3 : deletePicture(OutPic);
  free(OriginIMG);
  free(NumberOfPulses);
  free(tempIMG);
  free(RIMG);
  free(lowIMG);
  free(temp_PA);
  free(PLIMG);
  free(temp_PL);
  free(temp_LSP);
 free(dec_mpcIMG);
 dispose_1 : deletePicture(OriginPic);
 dispose_0 : exit(0);
}

