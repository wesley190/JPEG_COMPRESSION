#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#define PI 3.141592653589793f

typedef struct _wav {
	int fs;
	char header[78];
	size_t length;
	unsigned char *LChannel;
	unsigned char *RChannel;
} wav;

int wav_read_fn(char *fn, wav *p_wav)
{
	//char header[78];
	short temp = 0;
	size_t i = 0;

	FILE *fp = fopen(fn, "rb");
	if(fp==NULL) {
		fprintf(stderr, "cannot read %s\n", fn);
		return 0;
	}
	fread(p_wav->header, sizeof(char), 78, fp);
	while( !feof(fp) ) {
		fread(&temp, sizeof(char), 1, fp);
		i++;
	}
	p_wav->length = i / 2;
	p_wav->LChannel = (unsigned char *) calloc(p_wav->length, sizeof(char));
	if( p_wav->LChannel==NULL ) {
		fprintf(stderr, "cannot allocate memory for LChannel in wav_read_fn\n");
		fclose(fp);
		return 0;
	}
	p_wav->RChannel = (unsigned char *) calloc(p_wav->length, sizeof(char));
	if( p_wav->RChannel==NULL ) {
		fprintf(stderr, "cannot allocate memory for RChannel in wav_read_fn\n");
		fclose(fp);
		return 0;
	}
	fseek(fp, 78, SEEK_SET);
	for(i=0;i<p_wav->length;i++) {
		fread(p_wav->LChannel+i, sizeof(char), 1, fp);
		fread(p_wav->RChannel+i, sizeof(char), 1, fp);
	}
	fclose(fp);
	return 1;
}

void wav_free(wav *p_wav)
{
	free(p_wav->LChannel);
	free(p_wav->RChannel);
}
int main(int argc, char **argv)
{
	char *fn_in = argv[1];
	char *C_xL = argv[2];
	char *C_xR = argv[3];
	char *C_yL = argv[4];
	char *C_yR = argv[5];
    char *B_xL = argv[6];
	char *B_xR = argv[7];
	char *B_yL = argv[8];
	char *B_yR = argv[9];
	char *rate = argv[10];

	wav wavin;
	
	int n = 0;
	int k,i,j;	
	int num;
	float p;
	int ceightL = 0,ceightR = 0;
	int count1 = 0,count2 = 0;
	unsigned char cl = 0,cr = 0;
	float rateL = 0,rateR = 0;

	// read wav
	if( wav_read_fn(fn_in, &wavin) == 0 ) {
		fprintf(stderr, "cannot read wav file %s\n", fn_in);
		exit(1);
	}

	FILE *c_xL = fopen(C_xL,"r");
	FILE *c_xR = fopen(C_xR,"r"); 
    FILE *bs_xL = fopen(B_xL,"w");
    FILE *bs_xR = fopen(B_xR,"w");

	unsigned char **ptr_xL = (unsigned char**)malloc(sizeof(unsigned char*)*256);
	unsigned char **ptr_xR = (unsigned char**)malloc(sizeof(unsigned char*)*256);
	for(i = 0;i<256;i++){
		ptr_xL[i] = (unsigned char*)malloc(sizeof(unsigned char)*40);
		ptr_xR[i] = (unsigned char*)malloc(sizeof(unsigned char)*40);
	}


	for(i = 0;i < 256;i++){
		fscanf(c_xL,"%d %f %s",&num,&p,ptr_xL[i]);
		fscanf(c_xR,"%d %f %s",&num,&p,ptr_xR[i]);
		
	}

	for(i = 0;i < wavin.length;i++){
		k = wavin.LChannel[i];
		n = (int)strlen(ptr_xL[k]);
		for(j = 0;j<n;j++){
			if(ptr_xL[k][j] == '1'){
				cl = cl<<1;
				cl = cl+1;
				ceightL++;
				count1++;
			}
			else{
				cl = cl<<1;
				ceightL++;
				count1++;
			}
			if(ceightL == 8){
				ceightL = 0;
				fprintf(bs_xL,"%c",cl);
				cl = 0;
			}
		}
			
		k = wavin.RChannel[i];
		n = (int)strlen(ptr_xR[k]);
		for(j = 0;j<n;j++){
			if(ptr_xR[k][j] == '1'){
				cr = cr<<1;
				cr = cr+1;
				ceightR++;
				count2++;
			}
			else{
				cr = cr<<1;
				ceightR++;
				count2++;
			}
			if(ceightR == 8){
				ceightR = 0;
				fprintf(bs_xR,"%c",cr);
				cr = 0;
			}
		}
	
	}
	for(j = 0;j<(8-ceightL);j++){
		cl = cl<<1;
	}
	fprintf(bs_xL,"%c",cl);
	for(j = 0;j<(8-ceightR);j++){
		cr = cr<<1;
	}
	fprintf(bs_xR,"%c",cr);

	FILE *ra = fopen(rate,"w+");
	rateL = (float)count1/(wavin.length*8);
	rateR = (float)count2/(wavin.length*8);
	fprintf(ra,"R(x_L) = %.15f\n",rateL);
	fprintf(ra,"R(x_R) = %.15f\n",rateR);

	for(i = 0;i<255;i++){
		free(ptr_xL[i]);
		free(ptr_xR[i]);
	}
	free(ptr_xL);
	free(ptr_xR);


	FILE *c_yL = fopen(C_yL,"r");
	FILE *c_yR = fopen(C_yR,"r"); 
    FILE *bs_yL = fopen(B_yL,"w");
    FILE *bs_yR = fopen(B_yR,"w");

	int *diffL = calloc(wavin.length,sizeof(int));
	int *diffR = calloc(wavin.length,sizeof(int));
	for(i = 0;i < wavin.length;i++){
		if(i == 0){
			diffL[i] = 0;
			diffR[i] = 0;
		}
		else{
			diffL[i] = (wavin.LChannel[i] - '0') - (wavin.LChannel[i-1] - '0');
			diffR[i] = (wavin.RChannel[i] - '0') - (wavin.RChannel[i-1] - '0');
		}
	} 

	unsigned char **ptr_yL = (unsigned char**)malloc(sizeof(unsigned char*)*511);
	unsigned char **ptr_yR = (unsigned char**)malloc(sizeof(unsigned char*)*511);
	for(i = 0;i<511;i++){
		ptr_yL[i] = (unsigned char*)malloc(sizeof(unsigned char)*40);
		ptr_yR[i] = (unsigned char*)malloc(sizeof(unsigned char)*40);
	} 
	
	count1 = 0;count2 = 0;
	ceightL = 0;ceightR = 0;
	cl = 0;cr = 0;

	for(i = 0;i < 511;i++){
		fscanf(c_yL,"%d %f %s",&num,&p,ptr_yL[i]);
		fscanf(c_yR,"%d %f %s",&num,&p,ptr_yR[i]);
	}
	
	for(i = 0;i < wavin.length;i++){
		k = diffL[i]+255;
		n = (int)strlen(ptr_yL[k]);
		for(j = 0;j<n;j++){
			if(ptr_yL[k][j] == '1'){
				cl = cl<<1;
				cl = cl+1;
				ceightL++;
				count1++;
			}
			else{
				cl = cl<<1;
				ceightL++;
				count1++;
			}
			if(ceightL == 8){
				ceightL = 0;
				fprintf(bs_yL,"%c",cl);
				cl = 0;
			}
		}
			
		k = diffR[i]+255;
		n = (int)strlen(ptr_yR[k]);
		for(j = 0;j<n;j++){
			if(ptr_yR[k][j] == '1'){
				cr = cr<<1;
				cr = cr+1;
				ceightR++;
				count2++;
			}
			else{
				cr = cr<<1;
				ceightR++;
				count2++;
			}
			if(ceightR == 8){
				ceightR = 0;
				fprintf(bs_yR,"%c",cr);
				cr = 0;
			}
		}
	}
	for(j = 0;j<(8-ceightL);j++){
		cl = cl<<1;
	}
	fprintf(bs_yL,"%c",cl);
	for(j = 0;j<(8-ceightR);j++){
		cr = cr<<1;
	}
	fprintf(bs_yR,"%c",cr);

	rateL = (float)count1/(wavin.length*8);
	rateR = (float)count2/(wavin.length*8);
	fprintf(ra,"R(y_L) = %.15f\n",rateL);
	fprintf(ra,"R(y_R) = %.15f\n",rateR);


	for(i = 0;i<511;i++){
		free(ptr_yL[i]);
		free(ptr_yR[i]);
	}
	free(ptr_yL);
	free(ptr_yR);

		

	wav_free(&wavin);
	

	fclose(c_xL);
	fclose(c_xR);
	fclose(c_yL);
	fclose(c_yR);
	fclose(bs_xL);
	fclose(bs_xR);
	fclose(bs_yL);
	fclose(bs_yR);
	fclose(ra);
	free(diffR);
	free(diffL);
}