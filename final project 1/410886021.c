#include <stdio.h>
#include <stdlib.h>
#include <math.h>


/*construct a structure of BMP header*/
typedef struct Bmpheader {
	char identifier[2]; // 0x0000
	unsigned int filesize; // 0x0002
	unsigned short reserved; // 0x0006
	unsigned short reserved2;
	unsigned int bitmap_dataoffset; // 0x000A
	unsigned int bitmap_headersize; // 0x000E
	unsigned int width; // 0x0012
	unsigned int height; // 0x0016
	unsigned short planes; // 0x001A
	unsigned short bits_perpixel; // 0x001C
	unsigned int compression; // 0x001E
	unsigned int bitmap_datasize; // 0x0022
	unsigned int hresolution; // 0x0026
	unsigned int vresolution; // 0x002A
	unsigned int usedcolors; // 0x002E
	unsigned int importantcolors; // 0x0032
	//unsigned int palette; // 0x0036
} Bitmap;

/*construct a structure of RGB*/
typedef struct _RGB{
	unsigned char R;
	unsigned char G;
	unsigned char B;
} RGB;

/*construct a structure of YCbCr*/
typedef struct _YCBCR{
	double Y;
	double Cb;
	double Cr;
} YCBCR;

/*construct a structure of YCbCr*/
typedef struct YCBCR{
	int Y;//設定為quantization後的int type
	int Cb;
	int Cr;
} YCbCr;

/*read header*/
void readheader(FILE* fp, Bitmap *x) {
	fread(&x->identifier, sizeof(x->identifier), 1, fp);
	fread(&x->filesize, sizeof(x->filesize), 1, fp);
	fread(&x->reserved, sizeof(x->reserved), 1, fp);
	fread(&x->reserved2, sizeof(x->reserved2), 1, fp);
	fread(&x->bitmap_dataoffset, sizeof(x->bitmap_dataoffset), 1, fp);
	fread(&x->bitmap_headersize, sizeof(x->bitmap_headersize), 1, fp);
	fread(&x->width, sizeof(x->width), 1, fp);
	fread(&x->height, sizeof(x->height), 1, fp);
	fread(&x->planes, sizeof(x->planes), 1, fp);
	fread(&x->bits_perpixel, sizeof(x->bits_perpixel), 1, fp);
	fread(&x->compression, sizeof(x->compression), 1, fp);
	fread(&x->bitmap_datasize, sizeof(x->bitmap_datasize), 1, fp);
	fread(&x->hresolution, sizeof(x->hresolution), 1, fp);
	fread(&x->vresolution, sizeof(x->vresolution), 1, fp);
	fread(&x->usedcolors, sizeof(x->usedcolors), 1, fp);
	fread(&x->importantcolors, sizeof(x->importantcolors), 1, fp);
}

/*input data without header into RGB structure*/
void RGBinput(FILE* fp, RGB **array, int H, int W,int skip){
	int i, j;
	char skip_buf[3] ={0,0,0};
	for (i = 0; i<H; i++){
		for (j = 0; j<W; j++){
			fread(&array[i][j].B,1,1,fp);
			fread(&array[i][j].G,1,1,fp);
			fread(&array[i][j].R,1,1,fp);
		}
		if (skip != 0) fread(skip_buf, skip, 1, fp);
	}
}

//為DCT預備的cosine查表
void cos_table(double **cosine){
	int i,j;
	for(i=0;i<8;i++){
		for(j=0;j<8;j++){
			cosine[i][j] = cos((2*i+1)*j*M_PI/16);
		}
	}
}

void writeheader(char *filename, Bitmap *bmpheader, RGB **RGB,int skip){
	FILE *fp;
	fp = fopen(filename, "wb");
	char skip_buf[3] = { 0, 0, 0 };
	
	if(fwrite(bmpheader->identifier, 1, 2, fp)==0||
		fwrite(&bmpheader->filesize, 4, 1, fp)==0||
		fwrite(&bmpheader->reserved, 2, 1, fp)==0||
		fwrite(&bmpheader->reserved2, 2, 1, fp)==0||
		fwrite(&bmpheader->bitmap_dataoffset, 4, 1, fp)==0||
		fwrite(&bmpheader->bitmap_headersize, 4, 1, fp)==0||
		fwrite(&bmpheader->width, 4, 1, fp)==0||
		fwrite(&bmpheader->height, 4, 1, fp)==0||
		fwrite(&bmpheader->planes, 2, 1, fp)==0||
		fwrite(&bmpheader->bits_perpixel, 2, 1, fp)==0||
		fwrite(&bmpheader->compression, 4, 1, fp)==0||
		fwrite(&bmpheader->bitmap_datasize, 4, 1, fp)==0||
		fwrite(&bmpheader->hresolution, 4, 1, fp)==0||
		fwrite(&bmpheader->vresolution, 4, 1, fp)==0||
		fwrite(&bmpheader->usedcolors, 4, 1, fp)==0||
		fwrite(&bmpheader->importantcolors, 4, 1, fp)==0)
		printf("write header error!");
	
	// bitmap data
	int i, j;
	for (i = 0; i< bmpheader->height; i++){
		for (j = 0; j<bmpheader->width; j++){
			fwrite(&RGB[i][j].B, sizeof(char), 1, fp);
			fwrite(&RGB[i][j].G, sizeof(char), 1, fp);
			fwrite(&RGB[i][j].R, sizeof(char), 1, fp);
		}
		if (skip != 0)  fwrite(skip_buf, skip, 1, fp); 
	}
	fclose(fp);
	
}
int main(int argc, char *argv[]){
	int i, j, r, c, u, v;
	FILE *fp_in;
	FILE *fp_out;

	char* fn_in= argv[1];
	char* fn_out = argv[2];
	fp_in = fopen(fn_in, "rb");
	if (!fp_in) printf("Fail to open %s as input file!\n", fn_in);
	fp_out = fopen(fn_out, "wb");
	if (!fp_out) printf("Fail to open %s as output file!\n", fn_out);
	
	//read header
	Bitmap header;
	readheader(fp_in, &header);
	
	int height = header.height;
	int width = header.width;
	printf("Image size: Width= %d x Height= %d pixels, total %d kb\n", width, height, width*height * 3/1024);
	
	// skip paddings at the end of each image line
	int skip = (4 - (header.width * 3) % 4);
	if (skip == 4) skip = 0;
	printf("Image line skip= %d bytes\n", skip);
	int H=height,W=width;
	
	//create 2D matrix for RGB
	RGB **array = (RGB**)calloc(H,sizeof(RGB*));
	for (i = 0; i < H; i++) 
		array[i] = (RGB*)calloc(W,sizeof(RGB)); 
	
	RGBinput(fp_in, array, H, W, skip);
	
	//create 2D matrix for YCbCr
	YCBCR **a = (YCBCR**)calloc(H,sizeof(YCBCR*));
	for ( i = 0; i < H; i++) 
		a[i] = (YCBCR*)calloc(W,sizeof(YCBCR));
	
	//RGB to YCbCr
	for (i = 0; i<H; i++){
		for (j = 0; j<W; j++){
			a[i][j].Y = 0.299*array[i][j].R + 0.587*array[i][j].G + 0.114*array[i][j].B;
			a[i][j].Cb = 0.564*(array[i][j].B-a[i][j].Y);
			a[i][j].Cr = 0.713*(array[i][j].R-a[i][j].Y);
		}
	}
	// 8x8, if not multiples of 8, then bitmap padded, i.e. one more block
    if(height%8==0) H=height;
    else H=floor(height/8+1)*8;
    if(width%8==0) W=width;
    else W=floor(width/8+1)*8;
	int m = H / 8;
	int n = W / 8;
	printf("# of 8x8 blocks: W= %d x H= %d blocks, total %d blocks\n", n, m, n*m);
	
	YCBCR **input = (YCBCR**)calloc(H,sizeof(YCBCR*));
	for ( i = 0; i < H; i++) 
		input[i] = (YCBCR*)calloc(W,sizeof(YCBCR));

	for(i=0;i<H;i++){
        for(j=0;j<W;j++){
            if(i<height&&j<width){
                input[i][j].Y = a[i][j].Y;
                input[i][j].Cb = a[i][j].Cb;
                input[i][j].Cr = a[i][j].Cr;
            }
            else{
                input[i][j].Y = 0;
                input[i][j].Cb = 0;
                input[i][j].Cr = 0;
            }
        }
    }
	
	//create 2D output matrix 
	YCBCR **q_out = (YCBCR**)calloc(H,sizeof(YCBCR*));
	for ( i = 0; i < H; i++) 
		q_out[i] = (YCBCR*)calloc(W,sizeof(YCBCR));
	
	for (i = 0; i < height; i++) 
		free(array[i]);
	free(array);
	for (i = 0; i < height; i++) 
		free(a[i]);
	free(a);

	//cosine table 
	double **cosine = (double**)calloc(8,sizeof(double*));
	for (i = 0; i < 8; i++) 
		cosine[i] = (double*)calloc(8,sizeof(double));
	cos_table(cosine);

	//預備Quantization table，Q1給影響力較大的Y，Q2給Cb,Cr
	int Q1[8][8]={{16,11,10,16,24,40,51,61},
				 {12,12,14,19,26,58,60,55},
				 {14,13,16,24,40,57,69,56},
				 {14,17,22,29,51,87,80,62},
				 {18,22,37,56,68,109,103,77},
				 {24,35,55,64,81,104,113,92},
				 {49,64,78,87,103,121,120,101},
				 {72,92,95,98,112,100,103,99}};
	
	int Q2[8][8]={
    		17,  18,  24,  47,  99,  99,  99,  99,
			18,  21,  26,  66,  99,  99,  99,  99,
			24,  26,  56,  99,  99,  99,  99,  99,
			47,  66,  99,  99,  99,  99,  99,  99,
			99,  99,  99,  99,  99,  99,  99,  99,
			99,  99,  99,  99,  99,  99,  99,  99,
			99,  99,  99,  99,  99,  99,  99,  99,
			99,  99,  99,  99,  99,  99,  99,  99,};	
					 
	
	double temp1=0.0, temp2=0.0, temp3=0.0,k=0.0, temp7=0.0, temp8=0.0, temp9=0.0;
				  	   
	for (i = 0; i<m; i++){
		for (j = 0; j<n; j++){
			for(u = 0; u < 8; u++){
				for(v = 0; v < 8; v++){
					//blocking and DCT
					for(r = 0; r < 8; r++){
						for(c = 0; c < 8; c++){
							temp1 += input[i*8+r][j*8+c].Y * cosine[r][u] * cosine[c][v];
							temp2 += input[i*8+r][j*8+c].Cb * cosine[r][u] * cosine[c][v];
							temp3 += input[i*8+r][j*8+c].Cr * cosine[r][u] * cosine[c][v];
						}
					}
					k=1.0;//B[k]
					if(u==0) k = k/sqrt(2);
					if (v==0) k = k/sqrt(2);
					q_out[i*8+u][j*8+v].Y  = k / 4 * temp1;//(4/HW)^(1/2) = (1/16)^(1/2) = 1/4
					q_out[i*8+u][j*8+v].Cb  = k / 4 * temp2;
					q_out[i*8+u][j*8+v].Cr  = k / 4 * temp3; 
					temp1 = 0.0;
					temp2 = 0.0;
					temp3 = 0.0;
				}
			}
			//quantization
			for(u = 0; u < 8; u++){
				for(v = 0; v < 8; v++){
					q_out[i*8+u][j*8+v].Y = round(q_out[i*8+u][j*8+v].Y / Q1[u][v]);
					q_out[i*8+u][j*8+v].Cb = round(q_out[i*8+u][j*8+v].Cb / Q2[u][v]);
					q_out[i*8+u][j*8+v].Cr = round(q_out[i*8+u][j*8+v].Cr / Q2[u][v]);
				}
			}
		}
	} 
	for (i = 0; i < H; i++) 
		free(input[i]);
	free(input);

	//create 2D output matrix 
	YCbCr **deq_out = (YCbCr**)calloc(H,sizeof(YCbCr*));
	for ( i = 0; i < H; i++) 
		deq_out[i] = (YCbCr*)calloc(W,sizeof(YCbCr));

	for (i = 0; i<m; i++){
		for (j = 0; j<n; j++){		
			//dequantization
			for(u = 0; u < 8; u++){
				for(v = 0; v < 8; v++){
					q_out[i*8+u][j*8+v].Y = q_out[i*8+u][j*8+v].Y * Q1[u][v];
					q_out[i*8+u][j*8+v].Cb = q_out[i*8+u][j*8+v].Cb * Q2[u][v];
					q_out[i*8+u][j*8+v].Cr = q_out[i*8+u][j*8+v].Cr * Q2[u][v];
				}
			}
			//IDCT
			for(u = 0; u < 8; u++){
				for(v = 0; v < 8; v++){
					for(r = 0; r < 8; r++){
						for(c = 0; c < 8; c++){
							k=1.0;
							if(r==0) k = k/sqrt(2);
							if (c==0) k = k/sqrt(2);
							temp7 += q_out[i*8+r][j*8+c].Y * cosine[u][r] * cosine[v][c] * k / 4;
							temp8 += q_out[i*8+r][j*8+c].Cb * cosine[u][r] * cosine[v][c] * k / 4;
							temp9 += q_out[i*8+r][j*8+c].Cr * cosine[u][r] * cosine[v][c] * k / 4;
						}
					}
					deq_out[i*8+u][j*8+v].Y = temp7;
					deq_out[i*8+u][j*8+v].Cb = temp8;
					deq_out[i*8+u][j*8+v].Cr = temp9;
					temp7 = 0.0;
					temp8 = 0.0;
					temp9 = 0.0;
				}
			}	
		}
	}

	for (i = 0; i < H; i++) 
		free(q_out[i]);
	free(q_out);
	for (i = 0; i < 8; i++) 
		free(cosine[i]);
	free(cosine);
	
	//create 2D matrix for RGB
	RGB **output = (RGB**)calloc(H,sizeof(RGB*));
	for (i = 0; i < H; i++) 
		output[i] = (RGB*)calloc(W,sizeof(RGB)); 
		
	double t = 0;
	unsigned char R=0,G=0,B=0;
	//YCbCr to RGB
	for (i = 0; i<H; i++){
		for (j = 0; j<W; j++){
			//預防R值溢位
			t = deq_out[i][j].Y + 1.402 * (deq_out[i][j].Cr);
			if(t >= 255) R = 255;
			else if(t < 0) R = 0;
			else R = (unsigned char)t;
			
			//預防G值溢位
			t = deq_out[i][j].Y - 0.34414 * (deq_out[i][j].Cb) - 0.71414*(deq_out[i][j].Cr);
			if(t >= 255) G = 255;
			else if(t < 0) G = 0; 
			else G = (unsigned char)t;
			
			//預防B值溢位
			t = deq_out[i][j].Y + 1.772 * (deq_out[i][j].Cb);
			if(t >= 255) B = 255;
			else if(t < 0) B = 0;
			else B = (unsigned char)t;
			
			output[i][j].R = R;
			output[i][j].G = G;
			output[i][j].B = B;
		}
	}
	writeheader(fn_out, &header, output,skip);

	for (i = 0; i < H; i++) 
		free(deq_out[i]);
	free(deq_out);
	for (i = 0; i < H; i++) 
		free(output[i]);
	free(output);
	fclose(fp_in);
	fclose(fp_out);
}
/*for(i = 0;i < wavin.length;i++){
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
	}*/