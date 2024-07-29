#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <memory.h>

#define FS 48000.0f
#define FL 1000.0f
#define FH 4000.0f
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

int wav_save_fn(char *fn, wav *p_wav)
{
	FILE *fp = fopen(fn, "wb");
	size_t i;
	if(fp==NULL) {
		fprintf(stderr, "cannot save %s\n", fn);
		return 0;
	}
	fwrite(p_wav->header, sizeof(char), 78, fp);
	for(i=0;i<p_wav->length;i++) {
		fwrite(p_wav->LChannel+i, sizeof(char), 1, fp);
		fwrite(p_wav->RChannel+i, sizeof(char), 1, fp);
	}
	fclose(fp);
	return 1;
}

int wav_init(size_t length, wav *p_wav)
{
	p_wav->length = length;
	p_wav->LChannel = (char *) calloc(p_wav->length, sizeof(char));
	if( p_wav->LChannel==NULL ) {
		fprintf(stderr, "cannot allocate memory for LChannel in wav_read_fn\n");
		return 0;
	}
	p_wav->RChannel = (char *) calloc(p_wav->length, sizeof(char));
	if( p_wav->RChannel==NULL ) {
		fprintf(stderr, "cannot allocate memory for RChannel in wav_read_fn\n");
		return 0;
	}
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
	char *P_xL = argv[2];
	char *P_xR = argv[3];
	char *P_yL = argv[4];
	char *P_yR = argv[5];
	char *etppl = argv[6];

	wav wavin;
	
	
	int n = 0;
	int k,i;
	int countL = 0,countR = 0;
	float e_xL = 0,e_xR = 0,e_yL =0,e_yR = 0;
	float p_xL = 0,p_xR = 0,p_yL =0,p_yR = 0;


	// read wav
	if( wav_read_fn(fn_in, &wavin) == 0 ) {
		fprintf(stderr, "cannot read wav file %s\n", fn_in);
		exit(1);
	}

	FILE *pmf_xL;
	FILE *pmf_xR; 
	pmf_xL = fopen(P_xL,"w+");
	pmf_xR = fopen(P_xR,"w+");

	float *numXL = calloc(1000,sizeof(float));
	float *numXR = calloc(1000,sizeof(float));
	for(i = 0;i < 256;i++){
		countL = 0;
		countR = 0;
		for(k = 0;k < wavin.length;k++){
			if(wavin.LChannel[k] == i) countL++;	//找i值在Channel出現次數
			if(wavin.RChannel[k] == i) countR++;
		}
		numXL[i] = (float)countL/(float)wavin.length;	//產生PMF
		numXR[i] = (float)countR/(float)wavin.length;
		if(numXL[i] != 0) e_xL -= numXL[i]*log2f(numXL[i]);	//累加產生亂度
		if(numXR[i] != 0) e_xR -= numXR[i]*log2f(numXR[i]);
	}
	for(i = 0;i < 256;i++){
		fprintf(pmf_xL,"%d %.15f\n",i,numXL[i]);
		fprintf(pmf_xR,"%d %.15f\n",i,numXR[i]);
	}

	FILE *pmf_yL;
	FILE *pmf_yR;
	pmf_yL = fopen(P_yL,"w+");
	pmf_yR = fopen(P_yR,"w+");

	float *numYL = calloc(1000,sizeof(float));
	float *numYR = calloc(1000,sizeof(float));
	int *diffL = calloc(wavin.length,sizeof(int));
	int *diffR = calloc(wavin.length,sizeof(int));
	for(i = 0;i < wavin.length;i++){
		if(i == 0){
			diffL[i] = 0;
			diffR[i] = 0;
		}
		else{
			diffL[i] = (wavin.LChannel[i] - '0') - (wavin.LChannel[i-1] - '0');	//將差值儲存在diff[]中
			diffR[i] = (wavin.RChannel[i] - '0') - (wavin.RChannel[i-1] - '0');
		}
	} 
	for(i = -255;i < 256;i++){
		countL = 0;
		countR = 0;
		for(k = 0;k < wavin.length;k++){
			if(diffL[k] == i) countL++;
			if(diffR[k] == i) countR++;
		}
		n = i+255;
		numYL[n] = (float)countL/(float)wavin.length;
		numYR[n] = (float)countR/(float)wavin.length;
		if(numYL[n] != 0) e_yL -= numYL[n]*log2f(numYL[n]);
		if(numYR[n] != 0) e_yR -= numYR[n]*log2f(numYR[n]);
	}
	for(i = -255;i < 256;i++){
		fprintf(pmf_yL,"%d %.15f\n",i,numYL[i+255]);
		fprintf(pmf_yR,"%d %.15f\n",i,numYR[i+255]);
	}

	FILE *enppl = fopen(etppl,"w+");

	p_xL = powf(2,e_xL);
	p_xR = powf(2,e_xR);
	p_yL = powf(2,e_yL);
	p_yR = powf(2,e_yR);
	fprintf(enppl,"H(x_L) = %.15f\n",e_xL);	//輸出entropy及perplexity
	fprintf(enppl,"PPL(x_L) = %.15f\n",p_xL);
	fprintf(enppl,"H(x_R) = %.15f\n",e_xR);
	fprintf(enppl,"PPL(x_R) = %.15f\n",p_xR);
	fprintf(enppl,"H(y_L) = %.15f\n",e_yL);
	fprintf(enppl,"PPL(y_L) = %.15f\n",p_yL);
	fprintf(enppl,"H(y_R) = %.15f\n",e_yR);
	fprintf(enppl,"PPL(y_R) = %.15f\n",p_yR);

	wav_free(&wavin);
	

	fclose(pmf_xL);
	fclose(pmf_xR);
	fclose(pmf_yL);
	fclose(pmf_yR);
	fclose(enppl);
	free(numXR);
	free(numXL);
	free(numYL);
	free(numYR);
	free(diffR);
	free(diffL);
}