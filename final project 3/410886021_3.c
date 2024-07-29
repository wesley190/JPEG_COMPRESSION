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
void YCbCr_to_RGB(YCBCR **intput,RGB **output,int height,int width){
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
//convolution mask
void sharpen(YCBCR **input, YCBCR **output, int height, int width){
	int i,j,x,y,u,v;
	double temp1,out1;
	double filter[3][3]={
		0.0,-1.0,0.0,
		-1.0,5.0,-1.0,
		0.0,-1.0,0.0};
		
	for(i=0;i<height;i++){
        for(j=0;j<width;j++){
        	out1 = 0.0;
        	for(u=0;u<3;u++){
        		x = i-1+u;//convolution 位置
        		for(v=0;v<3;v++){
        			y = j-1+v;
        			if(x<0 || x>=height || y<0 || y>=width){
        				temp1 = 0.0;
					}
					else{
						temp1 = input[x][y].Y;
					}        			
					out1 += temp1 * filter[u][v];
				}
        	}
        	output[i][j].Y = out1;
        	output[i][j].Cb = input[i][j].Cb;
        	output[i][j].Cr = input[i][j].Cr;
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

    YCBCR **convol = (YCBCR**)calloc(H,sizeof(YCBCR*));
	for ( i = 0; i < H; i++) 
		convol[i] = (YCBCR*)calloc(W,sizeof(YCBCR));
	
    // RGB to YCbCr
    RGB_to_YCbCr(array,R2Y,H,W);
    sharpen(R2Y, convol, height, width);

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
                input[i][j].Y = convol[i][j].Y;
                input[i][j].Cb = convol[i][j].Cb;
                input[i][j].Cr = convol[i][j].Cr;
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
    for (i = 0; i < height; i++) 
		free(convol[i]);
	free(convol);

    RGB **output = (RGB**)calloc(H,sizeof(RGB*));
	for ( i = 0; i < H; i++) 
		output[i] = (RGB*)calloc(W,sizeof(RGB));
	//YCbCr to RGB	
	YCbCr_to_RGB(input, output, H, W);
	//write the file
	writeheader(fn_out, &header, output,skip);

    for (i = 0; i < H; i++) 
		free(input[i]);
	free(input);
    for (i = 0; i < H; i++) 
		free(output[i]);
	free(output);
	fclose(fp_in);
	fclose(fp_out);
    }