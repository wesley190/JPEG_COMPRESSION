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
	char header[44];
	size_t length;
	short *LChannel;
	short *RChannel;
} wav;

int wav_read_fn(char *fn, wav *p_wav)
{
	//char header[44];
	short temp = 0;
	size_t i = 0;

	FILE *fp = fopen(fn, "rb");
	if(fp==NULL) {
		fprintf(stderr, "cannot read %s\n", fn);
		return 0;
	}
	fread(p_wav->header, sizeof(char), 44, fp);
	while( !feof(fp) ) {
		fread(&temp, sizeof(short), 1, fp);
		i++;
	}
	p_wav->length = i / 2;
	p_wav->LChannel = (short *) calloc(p_wav->length, sizeof(short));
	if( p_wav->LChannel==NULL ) {
		fprintf(stderr, "cannot allocate memory for LChannel in wav_read_fn\n");
		fclose(fp);
		return 0;
	}
	p_wav->RChannel = (short *) calloc(p_wav->length, sizeof(short));
	if( p_wav->RChannel==NULL ) {
		fprintf(stderr, "cannot allocate memory for RChannel in wav_read_fn\n");
		fclose(fp);
		return 0;
	}
	fseek(fp, 44, SEEK_SET);
	for(i=0;i<p_wav->length;i++) {
		fread(p_wav->LChannel+i, sizeof(short), 1, fp);
		fread(p_wav->RChannel+i, sizeof(short), 1, fp);
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
	fwrite(p_wav->header, sizeof(char), 44, fp);
	for(i=0;i<p_wav->length;i++) {
		fwrite(p_wav->LChannel+i, sizeof(short), 1, fp);
		fwrite(p_wav->RChannel+i, sizeof(short), 1, fp);
	}
	fclose(fp);
	return 1;
}

int wav_init(size_t length, wav *p_wav)
{
	p_wav->length = length;
	p_wav->LChannel = (short *) calloc(p_wav->length, sizeof(short));
	if( p_wav->LChannel==NULL ) {
		fprintf(stderr, "cannot allocate memory for LChannel in wav_read_fn\n");
		return 0;
	}
	p_wav->RChannel = (short *) calloc(p_wav->length, sizeof(short));
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

/* hamming: for n=0,1,2,...N, length of N+1 */
float hamming(int N, int n)
{
	return 0.54 - 0.46 * cosf(2*PI*((float)(n))/((float)N-1.0));
}

float band_pass(int m, int n)
{
	float wh = 2*PI*FH/FS;
    float wl = 2*PI*FL/FS;
	if(n==m) {// L'Hopital's Rule
		return 2.0*(wh/PI - wl/PI);
	}
	else {
		return 2.0*(sinf(wh*((float)(n-m)))-sinf(wl*((float)(n-m))))/PI/((float)(n-m)) * hamming(2*m+1, n);
	}
}
float band_stop(int m, int n)
{
	float wh = 2*PI*FH/FS;
    float wl = 2*PI*FL/FS;
	if(n==m) {// L'Hopital's Rule
		return (1.0-(wh/PI - wl/PI));
	}
	else {
		return (-1.0)*(sinf(wh*((float)(n-m)))-sinf(wl*((float)(n-m))))/PI/((float)(n-m)) * hamming(2*m+1, n);
	}
}



int main(int argc, char **argv)
{
	int M = atoi(argv[1]);
	char* Lname = argv[2];
	char* Rname = argv[3];
	char* YLname = argv[4];
	char* YRname = argv[5];
	char *fn_in = argv[6];
	char *fn_out = argv[7];

	wav wavin;
	wav wavout;
	float *h_L = calloc((2*M+1),sizeof(float));
    float *h_R = calloc((2*M+1),sizeof(float));;
	int n = 0;
	float y = 0;
	int k;


	// read wav
	if( wav_read_fn(fn_in, &wavin) == 0 ) {
		fprintf(stderr, "cannot read wav file %s\n", fn_in);
		exit(1);
	}


	// construct low-pass filter
	for(n=0;n<(2*M+1);n++) {
		h_L[n] = band_pass(M, n);
        h_R[n] = band_stop(M, n);
	}

	FILE *hL;
	FILE *hR; 
	hL = fopen(Lname,"w+");
	hR = fopen(Rname,"w+");
    
	for(n=0;n<(2*M+1);n++) {
		fprintf(hL, "%.15e\n", h_L[n]);
		fprintf(hR, "%.15e\n", h_R[n]);
	}
    fclose(hL);
	fclose(hR);

	// filtering (convolution)
	if( wav_init(wavin.length, &wavout)==0 ) {
		exit(1);
	}

	for(n=0;n<wavin.length;n++) {
		y = 0;
		for(k=0;k<(2*M+1);k++) {
			if( (n-k)>=0 )
				y = y + h_L[k] * ((float)(wavin.LChannel[n-k]));
		}
		wavout.LChannel[n] = (short)(roundf(y));

		y = 0;
		for(k=0;k<(2*M+1);k++) {
			if( (n-k)>=0 )
				y = y + h_R[k] * ((float)(wavin.RChannel[n-k]));
		}
		wavout.RChannel[n] = (short)(roundf(y));
		
	}

	float Real = 0,Image = 0;
	float Real1 = 0,Image1 = 0;
	float Xn = 0,Xn1 = 0;
	float *X_l = calloc(1200,sizeof(float));
	float *X_r = calloc(1200,sizeof(float));

	FILE *YL = fopen(YLname,"w+");
	FILE *YR = fopen(YRname,"w+");

	for(k=0;k<1200;k++){
		X_l[k] = wavout.LChannel[30*48000+k]*hamming(1200,k);	//X[n] = x[m*M+n]*w[n]
		X_r[k] = wavout.RChannel[30*48000+k]*hamming(1200,k);
	}
	for(k=0;k<600;k++){//doing DFT
		Real=0;Image=0;
		Real1=0;Image1=0;
		for(n=0;n<1200;n++){
			Real += cos((2*M_PI/1200)*n*k)*X_l[n];//X[n]*e^(jwt) = X[n]*(cos(wt)-i*sin(wt))
			Image -= sin((2*M_PI/1200)*n*k)*X_l[n];
			Real1 += cos((2*M_PI/1200)*n*k)*X_r[n];
			Image1 -= sin((2*M_PI/1200)*n*k)*X_r[n];		
		}
		Xn =20*log10(sqrt(Real * Real + Image * Image));
		Xn1 =20*log10(sqrt(Real1 * Real1 + Image1 * Image1));
        
		fprintf( YL, "%.15e\n", Xn);
		fprintf( YR, "%.15e\n", Xn1);
	}
	memcpy(wavout.header, wavin.header, 44);


	// save wav
	if( wav_save_fn(fn_out, &wavout)==0) {
		fprintf(stderr, "cannot save %s\n", fn_out);
		exit(1);

	}
	wav_free(&wavin);
	wav_free(&wavout);

	fclose(hL);
	fclose(hR);
	fclose(YL);
	fclose(YR);
	free(h_L);
	free(h_R);
	free(X_l);
	free(X_r);
}