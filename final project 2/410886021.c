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
typedef struct YCBCR{
	int Y;
	int Cb;
	int Cr;
} YCbCr;

/*construct a structure of YCbCr*/
typedef struct _YCBCR{
	double Y;
	double Cb;
	double Cr;
} YCBCR;

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
void RGB_to_YCbCr(RGB **input,YCBCR **ouput,int height,int width){
    int i,j;
    for (i = 0; i<height; i++){
		for (j = 0; j<width; j++){
			ouput[i][j].Y = 0.299*input[i][j].R + 0.587*input[i][j].G + 0.114*input[i][j].B;
			ouput[i][j].Cb = 0.564*(input[i][j].B-ouput[i][j].Y);
			ouput[i][j].Cr = 0.713*(input[i][j].R-ouput[i][j].Y);
		}
	}
	printf("...RGB_to_YCbCr...\n");
	for(int u = 0;u<8;u++){
		for(int v = 0;v<8;v++){
			printf("%5.2f ",ouput[u][v].Y);
		}
		printf("\n");
	}
}
void YCbCr_to_RGB(YCbCr **intput,RGB **output,int height,int width){
    int i,j,tr = 0,tg =0,tb = 0;
	unsigned char R=0,G=0,B=0;
    for (i = 0; i<height; i++){
		for (j = 0; j<width; j++){
			tr = intput[i][j].Y + 1.402 * (intput[i][j].Cr);
			tg = intput[i][j].Y - 0.34414 * (intput[i][j].Cb) - 0.71414*(intput[i][j].Cr);
			tb = intput[i][j].Y + 1.772 * (intput[i][j].Cb);

			if(tr >= 255) R = 255;//預防R值溢位
			else if(tr < 0) R = 0;
			else R = (unsigned char)tr;
			
			if(tg >= 255) G = 255;//預防G值溢位
			else if(tg < 0) G = 0; 
			else G = (unsigned char)tg;
			
			if(tb >= 255) B = 255;//預防B值溢位
			else if(tb < 0) B = 0;
			else B = (unsigned char)tb;
			
			output[i][j].R = R;
			output[i][j].G = G;
			output[i][j].B = B;
		}
	}
}
void blocking_and_DCT(int blockh,int blockw,YCBCR **ycbcr,double **cosine,YCBCR **output){
    double temp1=0.0, temp2=0.0, temp3=0.0,k=0.0;
    int u,v,r,c;
    for(u = 0; u < 8; u++){
		for(v = 0; v < 8; v++){
			//blocking and DCT
			for(r = 0; r < 8; r++){
				for(c = 0; c < 8; c++){
					temp1 += ycbcr[blockh*8+r][blockw*8+c].Y * cosine[r][u] * cosine[c][v];
					temp2 += ycbcr[blockh*8+r][blockw*8+c].Cb * cosine[r][u] * cosine[c][v];
					temp3 += ycbcr[blockh*8+r][blockw*8+c].Cr * cosine[r][u] * cosine[c][v];
				}
			}
			k=1.0;//B[k]
			if(u==0) k = k/sqrt(2);
			if (v==0) k = k/sqrt(2);
			output[blockh*8+u][blockw*8+v].Y  = k / 4 * temp1;//(4/HW)^(1/2) = (1/16)^(1/2) = 1/4
			output[blockh*8+u][blockw*8+v].Cb  = k / 4 * temp2;
			output[blockh*8+u][blockw*8+v].Cr  = k / 4 * temp3; 
			temp1 = 0.0;
			temp2 = 0.0;
			temp3 = 0.0;

			if(blockh == 0 && blockw == 0){
				printf("%7.2f ",output[blockh*8+u][blockw*8+v].Y);
				if((v+1)%8 == 0)
					printf("\n");
			}
		}
	}
	
}
void Inverse_DCT(int blockh,int blockw,YCBCR **input,double **cosine,YCbCr **IDCT_out){
    double temp1=0.0, temp2=0.0, temp3=0.0,k=0.0;
    int u,v,r,c;
    for(u = 0; u < 8; u++){
		for(v = 0; v < 8; v++){
			for(r = 0; r < 8; r++){
				for(c = 0; c < 8; c++){
					k=1.0;//B[k]
					if(r==0) k = k/sqrt(2);
					if (c==0) k = k/sqrt(2);
					temp1 += input[blockh*8+r][blockw*8+c].Y * cosine[u][r] * cosine[v][c]* k / 4;//(4/HW)^(1/2) = (1/16)^(1/2) = 1/4
					temp2 += input[blockh*8+r][blockw*8+c].Cb * cosine[u][r] * cosine[v][c]* k / 4;
					temp3 += input[blockh*8+r][blockw*8+c].Cr * cosine[u][r] * cosine[v][c]* k / 4;
				}
			}
			IDCT_out[blockh*8+u][blockw*8+v].Y = temp1;
			IDCT_out[blockh*8+u][blockw*8+v].Cb = temp2;
			IDCT_out[blockh*8+u][blockw*8+v].Cr = temp3;
			temp1 = 0.0;
			temp2 = 0.0;
			temp3 = 0.0;
		}
	}
	
}
void quantization(int blockh,int blockw,YCBCR **q_out){
    int u,v;
    //預備Quantization table，Q1給影響力較大的Y，Q2給Cb,Cr
	int Q1[8][8]={{16,11,10,16,24,40,51,61},
				 {12,12,14,19,26,58,60,55},
				 {14,13,16,24,40,57,69,56},
				 {14,17,22,29,51,87,80,62},
				 {18,22,37,56,68,109,103,77},
				 {24,35,55,64,81,104,113,92},
				 {49,64,78,87,103,121,120,101},
				 {72,92,95,98,112,100,103,99}};
	
	int Q2[8][8]={{17,18,24,47,99,99,99,99},
			     {18,21,26,66,99,99,99,99},
			     {24,26,56,99,99,99,99,99},
			     {47,66,99,99,99,99,99,99},
			     {99,99,99,99,99,99,99,99},
			     {99,99,99,99,99,99,99,99},
			     {99,99,99,99,99,99,99,99},
			     {99,99,99,99,99,99,99,99,}};
    for(u = 0; u < 8; u++){
		for(v = 0; v < 8; v++){
			q_out[blockh*8+u][blockw*8+v].Y = round(q_out[blockh*8+u][blockw*8+v].Y / Q1[u][v]);
			q_out[blockh*8+u][blockw*8+v].Cb = round(q_out[blockh*8+u][blockw*8+v].Cb / Q2[u][v]);
			q_out[blockh*8+u][blockw*8+v].Cr = round(q_out[blockh*8+u][blockw*8+v].Cr / Q2[u][v]);
		}
	}         
}
void dequantization(int blockh,int blockw,YCBCR **deq_out){
    int u,v;
    int Q1[8][8]={{16,11,10,16,24,40,51,61},
				 {12,12,14,19,26,58,60,55},
				 {14,13,16,24,40,57,69,56},
				 {14,17,22,29,51,87,80,62},
				 {18,22,37,56,68,109,103,77},
				 {24,35,55,64,81,104,113,92},
				 {49,64,78,87,103,121,120,101},
				 {72,92,95,98,112,100,103,99}};
	
	int Q2[8][8]={{17,18,24,47,99,99,99,99},
			     {18,21,26,66,99,99,99,99},
			     {24,26,56,99,99,99,99,99},
			     {47,66,99,99,99,99,99,99},
			     {99,99,99,99,99,99,99,99},
			     {99,99,99,99,99,99,99,99},
			     {99,99,99,99,99,99,99,99},
			     {99,99,99,99,99,99,99,99,}};
    for(u = 0; u < 8; u++){
		for(v = 0; v < 8; v++){
			deq_out[blockh*8+u][blockw*8+v].Y = deq_out[blockh*8+u][blockw*8+v].Y * Q1[u][v];
			deq_out[blockh*8+u][blockw*8+v].Cb = deq_out[blockh*8+u][blockw*8+v].Cb * Q2[u][v];
			deq_out[blockh*8+u][blockw*8+v].Cr = deq_out[blockh*8+u][blockw*8+v].Cr * Q2[u][v];
		}
	}
}
void DC_DPCM(int block_height,int block_width,YCbCr **dpcm){
	int i=0,j=0;
	//DPCM for DC
	for(i = 0;i < block_height;i++){
		for(j = block_width-1; j > 0; j--){
			dpcm[i][j].Y = dpcm[i][j].Y - dpcm[i][j-1].Y;
			dpcm[i][j].Cb = dpcm[i][j].Cb - dpcm[i][j-1].Cb;
			dpcm[i][j].Cr = dpcm[i][j].Cr - dpcm[i][j-1].Cr;
		}
	}
	for(i = block_height-1; i > 0; i--){
		dpcm[i][0].Y = dpcm[i][0].Y - dpcm[i-1][0].Y;
		dpcm[i][0].Cb = dpcm[i][0].Cb - dpcm[i-1][0].Cb;
		dpcm[i][0].Cr = dpcm[i][0].Cr - dpcm[i-1][0].Cr;
	}
	printf("...DC_DPCM...\n");
	for(int u = 0;u<8;u++){
		for(int v = 0;v<8;v++){
			printf("%3d ",dpcm[u][v].Y);
		}
		printf("\n");
	}
}
void DC_IDPCM(int block_height,int block_width,YCbCr **Idpcm){
    int i,j;
    for(i = 1; i < block_height; i++){
		Idpcm[i][0].Y = Idpcm[i][0].Y + Idpcm[i-1][0].Y;
		Idpcm[i][0].Cb = Idpcm[i][0].Cb + Idpcm[i-1][0].Cb;
		Idpcm[i][0].Cr = Idpcm[i][0].Cr + Idpcm[i-1][0].Cr;
	}
	for(i = 0;i < block_height;i++){
		for(j = 1; j < block_width; j++){
			Idpcm[i][j].Y = Idpcm[i][j].Y + Idpcm[i][j-1].Y;
			Idpcm[i][j].Cb = Idpcm[i][j].Cb + Idpcm[i][j-1].Cb;
			Idpcm[i][j].Cr = Idpcm[i][j].Cr + Idpcm[i][j-1].Cr;
		}
	}
	printf("...DC_IDPCM...\n");
	for(int u = 0;u<8;u++){
		for(int v = 0;v<8;v++){
			printf("%3d ",Idpcm[u][v].Y);
		}
		printf("\n");
	}
}
void Zigzag(int blockh,int blockw,YCBCR **input,YCbCr *zig){
	int u=0;
	int zig_row[64]={0,1,0,0,1,2,3,2,1,0,0,1,2,3,4,5,
					 4,3,2,1,0,0,1,2,3,4,5,6,7,6,5,4,
					 3,2,1,0,1,2,3,4,5,6,7,7,6,5,4,3,
					 2,3,4,5,6,7,7,6,5,4,5,6,7,7,6,7
				 	};
	int zig_col[64]={0,0,1,2,1,0,0,1,2,3,4,3,2,1,0,0,
					 1,2,3,4,5,6,5,4,3,2,1,0,0,1,2,3,
					 4,5,6,7,7,6,5,4,3,2,1,2,3,4,5,6,
					 7,7,6,5,4,3,4,5,6,7,7,6,5,6,7,7
			 		 };
	for(u=0;u<64;u++) {
		zig[u].Y = input[blockh*8+zig_row[u]][blockw*8+zig_col[u]].Y;
		zig[u].Cb= input[blockh*8+zig_row[u]][blockw*8+zig_col[u]].Cb;
		zig[u].Cr = input[blockh*8+zig_row[u]][blockw*8+zig_col[u]].Cr;
	}
}
void Inverse_zigzag(int blockh,int blockw,YCbCr *zig,YCBCR **out){
    int u=0;
	int zig_row[64]={0,1,0,0,1,2,3,2,1,0,0,1,2,3,4,5,
					 4,3,2,1,0,0,1,2,3,4,5,6,7,6,5,4,
					 3,2,1,0,1,2,3,4,5,6,7,7,6,5,4,3,
					 2,3,4,5,6,7,7,6,5,4,5,6,7,7,6,7
				 	};
	int zig_col[64]={0,0,1,2,1,0,0,1,2,3,4,3,2,1,0,0,
					 1,2,3,4,5,6,5,4,3,2,1,0,0,1,2,3,
					 4,5,6,7,7,6,5,4,3,2,1,2,3,4,5,6,
					 7,7,6,5,4,3,4,5,6,7,7,6,5,6,7,7
			 		 };
    for(u=0;u<64;u++) {
		out[blockh*8+zig_row[u]][blockw*8+zig_col[u]].Y = zig[u].Y;
		out[blockh*8+zig_row[u]][blockw*8+zig_col[u]].Cb = zig[u].Cb;
    	out[blockh*8+zig_row[u]][blockw*8+zig_col[u]].Cr = zig[u].Cr;
	}
}
int count1=0,count2=0,count3=0,zero1=0,zero2=0,zero3=0,t1=0,t2=0,t3=0;
void AC_RLE(YCbCr *zig,YCbCr *RLE){
    int u;
    for(u=1;u<64;u++){
		if(zig[u].Y!=0){
			RLE[count1].Y = zero1;
			zero1 = 0;
			RLE[count1+1].Y = zig[u].Y;
			count1+=2;
		}
		else{
			zero1++;
		}
		if(zig[u].Cb!=0){			
			RLE[count2].Cb = zero2;
			zero2 = 0;
			RLE[count2+1].Cb = zig[u].Cb;
			count2+=2;
		}
		else{
			zero2++;
		}
		if(zig[u].Cr!=0){
			RLE[count3].Cr = zero3;
			zero3 = 0;
			RLE[count3+1].Cr = zig[u].Cr;
			count3+=2;
		}
		else{
			zero3++;
		}
	}
	RLE[count1].Y = 0;//最後補上(0,0);
	RLE[count2].Cb = 0;
	RLE[count3].Cr = 0;
	zero1 = 0;
	zero2 = 0;
	zero3 = 0;
	RLE[count1+1].Y = 0;
	RLE[count2+1].Cb = 0;
	RLE[count3+1].Cr = 0;
	count1+=2;
	count2+=2;
	count3+=2;
}
void AC_RLD(YCbCr *RLE,YCbCr *output){
    int u;
    for(u=1;u<64;u++){
	    //RLD for Y
		if(RLE[t1].Y==0&&RLE[t1+1].Y==0){
			output[u].Y = 0;
			if(u==63)
				t1+=2;
		}
		else if(RLE[t1].Y==0&&RLE[t1+1].Y!=0){
			output[u].Y = RLE[t1+1].Y;
			t1+=2;
			if(u==63){
				t1+=2;
			}
		}
		else{
			output[u].Y = 0;
			RLE[t1].Y-=1;
		}
		//RLD for Cb
		if(RLE[t2].Cb==0&&RLE[t2+1].Cb==0){
			output[u].Cb = 0;
			if(u==63)
				t2+=2;
		}
		else if(RLE[t2].Cb==0&&RLE[t2+1].Cb!=0){
			output[u].Cb = RLE[t2+1].Cb;
			t2+=2;
			if(u==63){
				t2+=2;
			}
		}
		else{
			output[u].Cb = 0;
			RLE[t2].Cb-=1;
		}
		//RLD for Cr
		if(RLE[t3].Cr==0&&RLE[t3+1].Cr==0){
			output[u].Cr = 0;
			if(u==63)
				t3+=2;
		}
		else if(RLE[t3].Cr==0&&RLE[t3+1].Cr!=0){
			output[u].Cr = RLE[t3+1].Cr;
			t3+=2;
			if(u==63){
				t3+=2;
			}
		}
		else{
			output[u].Cr = 0;
			RLE[t3].Cr-=1;	
		}
	}
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
	printf("Image size: Width: %d x Height=%d pixels, total %d kb\n", width, height, width*height * 3/1024);
	
	// skip paddings at the end of each image line
	int skip = (4 - (header.width * 3) % 4);
	if (skip == 4) skip = 0;
	printf("Image line skip=%d bytes\n", skip);
	int H=height,W=width;
	
	//create 2D matrix for RGB
	RGB **array = (RGB**)calloc(H,sizeof(RGB*));
	for (i = 0; i < H; i++) 
		array[i] = (RGB*)calloc(W,sizeof(RGB)); 
	
	RGBinput(fp_in, array, H, W, skip);
	
	//create 2D matrix for YCbCr
	YCBCR **R2Y = (YCBCR**)calloc(H,sizeof(YCBCR*));
	for ( i = 0; i < H; i++) 
		R2Y[i] = (YCBCR*)calloc(W,sizeof(YCBCR));
	
    // RGB to YCbCr
    RGB_to_YCbCr(array,R2Y,H,W);

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
                input[i][j].Y = R2Y[i][j].Y;
                input[i][j].Cb = R2Y[i][j].Cb;
                input[i][j].Cr = R2Y[i][j].Cr;
            }
            else{//將長寬不足整除8的部分補零
                input[i][j].Y = 0;
                input[i][j].Cb = 0;
                input[i][j].Cr = 0;
            }
        }
    }
	
    for (i = 0; i < height; i++) 
		free(array[i]);
	free(array);
	for (i = 0; i < height; i++) 
		free(R2Y[i]);
	free(R2Y);

	//create 2D output matrix 
	YCBCR **q_out = (YCBCR**)calloc(H,sizeof(YCBCR*));
	for ( i = 0; i < H; i++) 
		q_out[i] = (YCBCR*)calloc(W,sizeof(YCBCR));
	
	//cosine table 
	double **cosine = (double**)calloc(8,sizeof(double*));
	for (i = 0; i < 8; i++) 
		cosine[i] = (double*)calloc(8,sizeof(double));

    //準備做DCT
	cos_table(cosine);

	printf("...blocking_and_DCT...\n");  	   
	for (i = 0; i<m; i++){
		for (j = 0; j<n; j++){
			
            blocking_and_DCT(i,j,input,cosine,q_out);
			//quantization
            quantization(i,j,q_out);
		}
	}
	
	printf("...quantization...\n");
	for(u = 0;u<8;u++){
		for(v = 0;v<8;v++){
			printf("%5.2f ",q_out[u][v].Y);
		}
		printf("\n");
	} 
	//create 2D matrix 
	YCbCr **DC = (YCbCr**)calloc(m,sizeof(YCbCr*));
	for ( i = 0; i < m; i++) 
		DC[i] = (YCbCr*)calloc(n,sizeof(YCbCr));
	
	//write DC matrix
	for (i = 0; i<m; i++){
		for (j = 0; j<n; j++){
		    DC[i][j].Y = (int)q_out[i*8][j*8].Y;
			DC[i][j].Cb= (int)q_out[i*8][j*8].Cb;
			DC[i][j].Cr = (int)q_out[i*8][j*8].Cr;
		}
	}	
	DC_DPCM(m,n,DC);
	
	YCbCr *zigzag = calloc(64,sizeof(YCbCr));
	
	YCbCr *AC = calloc(H*W,sizeof(YCbCr)); 

	for (i = 0; i<m; i++){
		for (j = 0; j<n; j++){
			//zigzag
			Zigzag(i,j,q_out,zigzag);
			if(i == 0 && j == 0){
				printf("...Zigzag...\n");
				for(u = 0;u<64;u++){
					printf("%3d ",zigzag[u].Y);
					if((u+1)%8 == 0){
						printf("\n");
					}
				} 
			}
			//RLE AC
            AC_RLE(zigzag,AC);
			if(i == 0 && j == 0){
				printf("...AC_RLE...\n");
				for(u = 0;u<64;u++){
					printf("%3d ",AC[u].Y);
					if((u+1)%8 == 0){
						printf("\n");
					}
				}
			}
		}
	}

	//invert DPCM for DC
    DC_IDPCM(m,n,DC);

	for (i = 0; i<m; i++){
		for (j = 0; j<n; j++){

			zigzag[0].Y = DC[i][j].Y;//給予DC值
			zigzag[0].Cb = DC[i][j].Cb;
			zigzag[0].Cr = DC[i][j].Cr;
			
            //RLD of AC
            AC_RLD(AC,zigzag);
			if(i == 0 && j == 0){
				printf("...AC_RLD...\n");
				for(u = 0;u<64;u++){
					printf("%3d ",zigzag[u].Y);
					if((u+1)%8 == 0){
						printf("\n");
					}
				} 
			}
			//invert of zigzag
            Inverse_zigzag(i,j,zigzag,q_out);
		}
	}

	printf("...Inverse_zigzag...\n");
	for(u = 0;u<8;u++){
		for(v = 0;v<8;v++){
			printf("%5.2f ",q_out[u][v].Y);
		}
		printf("\n");
	}
	
	//create 2D output matrix 
	YCbCr **deq_out = (YCbCr**)calloc(H,sizeof(YCbCr*));
	for ( i = 0; i < H; i++) 
		deq_out[i] = (YCbCr*)calloc(W,sizeof(YCbCr));

	for (i = 0; i<m; i++){
		for (j = 0; j<n; j++){		

			//dequantization
            dequantization(i,j,q_out);
			//IDCT
            Inverse_DCT(i,j,q_out,cosine,deq_out);	
		}
	}

	printf("...Dequantization...\n");
	for(u = 0;u<8;u++){
		for(v = 0;v<8;v++){
			printf("%7.2f ",q_out[u][v].Y);
		}
		printf("\n");
	}

	printf("...Inverse_DCT...\n");
	for(u = 0;u<8;u++){
		for(v = 0;v<8;v++){
			printf("%d ",deq_out[u][v].Y);
		}
		printf("\n");
	}
	
    for(i = 0; i < m; i++)
        free(DC[i]);
    free(DC);
	for(i = 0; i < H; i++) 
		free(q_out[i]);
	free(q_out);
    for(i = 0; i < 8; i++) 
		free(cosine[i]);
	free(cosine);
    free(zigzag);
    free(AC);
	
	//create 2D matrix for RGB
	RGB **Y2R = (RGB**)calloc(H,sizeof(RGB*));
	for (i = 0; i < H; i++) 
		Y2R[i] = (RGB*)calloc(W,sizeof(RGB)); 
		
	//YCbCr to RGB
    YCbCr_to_RGB(deq_out,Y2R,H,W);

	writeheader(fn_out, &header, Y2R,skip);

    for (i = 0; i < H; i++) 
		free(deq_out[i]);
	free(deq_out);
	for (i = 0; i < H; i++) 
		free(Y2R[i]);
	free(Y2R);
	fclose(fp_in);
	fclose(fp_out);
}
