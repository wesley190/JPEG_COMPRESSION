## JPEG Compression

Use C to implement JPEG compression.

### Block diagram

![image](https://user-images.githubusercontent.com/128220508/226189874-4b4e13f0-ad6f-42a8-9c58-46bb58dfaa2f.png)

### 1. Read data

```js
void read_header(FILE *fp, Bitmap *x){
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
```

Read data : B -> G -> R

```js
void read_data(FILE *fp, ImgRGB **array, int H, int W, int skip){
    printf("\nread data...\n");
	int temp;
	char skip_buf[3];
	for(i=0;i<H;i++){
		for(j=0;j<W;j++){
			temp = fgetc(fp);
			array[i][j].B = temp;
			temp = fgetc(fp);
			array[i][j].G = temp;
			temp = fgetc(fp);
			array[i][j].R = temp;
		}
		if (skip != 0) fread(skip_buf, skip, 1, fp);
	}
	printf("finish\n");
}
```

### 2. RGB to YCbCr

- Y = 0.299*R + 0.578*G + 0.114\*B
- Cb = 0.564\*(B-Y)
- Cr = 0.713\*(R-Y)

```js
void YCbCr(ImgRGB **data_RGB, ImgYCbCr **data_YCbCr, int H, int W){
    printf("\nYCbCr...\n");
    for(i=0;i<H;i++){
        for(j=0;j<W;j++){
            for(k=0;k<8;k++){
                for(l=0;l<8;l++){
                    data_YCbCr[i][j].Y[k][l]=0.299*data_RGB[i*8+k][j*8+l].R + 0.578*data_RGB[i*8+k][j*8+l].G + 0.114*data_RGB[i*8+k][j*8+l].B;
                    data_YCbCr[i][j].Cb[k][l]=0.564*(data_RGB[i*8+k][j*8+l].B - data_YCbCr[i][j].Y[k][l]);
                    data_YCbCr[i][j].Cr[k][l]=0.713*(data_RGB[i*8+k][j*8+l].R - data_YCbCr[i][j].Y[k][l]);
                }
            }
        }
   }
   printf("finish\n");
}
```

#### Print data_YCbCr[0][0].Y for example

```js
for (k = 0; k < 8; k++) {
  for (l = 0; l < 8; l++) {
    printf("%3.3f ", data_YCbCr[0][0].Y[k][l]);
  }
}
```

![image](https://user-images.githubusercontent.com/128220508/226192611-a0ff1e6d-c3fb-4518-a077-8c0d56651762.png)

### 3. Discrete cosine transform

#### (1) Calculate cosine value

The values of cosine are periodic.

- 𝑐𝑜𝑠⁡(𝜋𝑢(2𝑟+1)/2𝐻)

```js
float cos1=0;
float cosA[8][8];

for(r=0;r<8;r++){
    for(u=0;u<8;u++){
        cosA[r][u]=cos((2*r+1)*u*PI/16);
        }
    }
```

- 𝑐𝑜𝑠⁡(𝜋𝑣(2𝑐+1)/2𝑊)

```js
float cos2=0;
float cosB[8][8];

for(c=0;c<8;c++){
    for(v=0;v<8;v++){
        cosB[c][v]=cos((2*c+1)*v*PI/16);
    }
}
```

#### (2) DCT

- Formula  
  ![image](https://user-images.githubusercontent.com/128220508/226192573-e593e2f0-a959-400f-9731-61c9cd927521.png)  
  The value must be in the range of -128~127.
- Judge u and v in 3 situations  
  ![image](https://user-images.githubusercontent.com/128220508/226192705-7482204d-96fa-47f7-b5d4-fb2bfe8500dc.png)

```js
void DCT(ImgYCbCr **data_YCbCr, ImgDCT **data_DCT, int H, int W){
    for(i=0;i<H;i++){
        for(j=0;j<W;j++){
            for(u=0;u<8;u++){
                for(v=0;v<8;v++){
                    sum_Y = 0;
                    sum_Cb = 0;
                    sum_Cr = 0;
                    for(r=0;r<8;r++){
                        for(c=0;c<8;c++){
                            if(u==0 && v==0){
                                sum_Y += (data_YCbCr[i][j].Y[r][c]-128)*cosA[r][u]*cosB[c][v]/2;
                                sum_Cb += (data_YCbCr[i][j].Cb[r][c]-128)*cosA[r][u]*cosB[c][v]/2;
                                sum_Cr += (data_YCbCr[i][j].Cr[r][c]-128)*cosA[r][u]*cosB[c][v]/2;
                            }

                            else if(u==0 || v==0){
                                sum_Y += (data_YCbCr[i][j].Y[r][c]-128)*cosA[r][u]*cosB[c][v]/sqrt(2);
                                sum_Cb += (data_YCbCr[i][j].Cb[r][c]-128)*cosA[r][u]*cosB[c][v]/sqrt(2);
                                sum_Cr += (data_YCbCr[i][j].Cr[r][c]-128)*cosA[r][u]*cosB[c][v]/sqrt(2);
                            }

                            else{
                                sum_Y += (data_YCbCr[i][j].Y[r][c]-128)*cosA[r][u]*cosB[c][v];
                                sum_Cb += (data_YCbCr[i][j].Cb[r][c]-128)*cosA[r][u]*cosB[c][v];
                                sum_Cr += (data_YCbCr[i][j].Cr[r][c]-128)*cosA[r][u]*cosB[c][v];
                            }
                        }
                    }
            data_DCT[i][j].Y[u][v]=sum_Y/4;
            data_DCT[i][j].Cb[u][v]=sum_Cb/4;
            data_DCT[i][j].Cr[u][v]=sum_Cr/4;
            }
        }
    }
}
```
#### Print data_DCT[0][0].Y for example
```js
for(k=0;k<8;k++){
    for(l=0;l<8;l++){
        printf("%3.3f ",data_DCT[0][0].Y[k][l]);
 		}
}
```

![image](https://user-images.githubusercontent.com/128220508/226196159-f0331103-991d-4290-b141-a737700a54d7.png)

### 4. Quantization

#### (1) Luminance Quantization Table

```js
int Q_Y[64]={
    16, 11, 10, 16, 24, 40, 51, 61,
    12, 12, 14, 19, 26, 58, 60, 55,
    14, 13, 16, 24, 40, 57, 69, 56,
    14, 17, 22, 29, 51, 87, 80, 62,
    18, 22, 37, 56, 68,109,103, 77,
    24, 35, 55, 64, 81,104,113, 92,
    49, 64, 78, 87,103,121,120,101,
    72, 92, 95, 98,112,100,103, 99 };
```

#### (2) Color quantizication table

```js
int Q_CbCr[64]={
    17, 18, 24, 47, 99, 99, 99, 99,
    18, 21, 26, 66, 99, 99, 99, 99,
    24, 26, 56, 99, 99, 99, 99, 99,
    47, 66, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99 };
```

#### Divide by quantization table

```js
void QUAN(ImgQUAN **data_QUAN, ImgDCT ** data_DCT, int H, int W){
    for(i=0;i<H;i++){
        for(j=0;j<W;j++){
            for(k=0;k<8;k++){
                for(l=0;l<8;l++){
                    data_QUAN[i][j].Y[k][l]=round(data_DCT[i][j].Y[k][l]/Q_Y[k*8+l]);
                    data_QUAN[i][j].Cb[k][l]=round(data_DCT[i][j].Cb[k][l]/Q_CbCr[k*8+l]);
                    data_QUAN[i][j].Cr[k][l]=round(data_DCT[i][j].Cr[k][l]/Q_CbCr[k*8+l]);
                }
            }
        }
    }
```

#### Print data_QUAN[0][0].Y for example

```js
for (k = 0; k < 8; k++) {
  for (l = 0; l < 8; l++) {
    printf("%3d ", data_QUAN[0][0].Y[k][l]);
  }
}
```

![image](https://user-images.githubusercontent.com/128220508/226194942-c28c4795-8240-45f3-8d62-51ba1d8c0888.png)

### 5. Zigzag

#### Zigzag table

```js
int z[64]={
     0,  1,  5,  6, 14, 15, 27, 28,
     2,  4,  7, 13, 16, 26, 29, 42,
     3,  8, 12, 17, 25, 30, 41, 43,
     9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63 };
```

![image](https://user-images.githubusercontent.com/128220508/226195001-9c68e34b-167c-4609-92a6-cca5ac25fb5e.png)

```js
void ZigZag(ImgZigZag **data_ZigZag, ImgQUAN ** data_QUAN, int H, int W){
    for(i=0;i<H;i++){
        for(j=0;j<W;j++){
            for(k=0;k<8;k++){
                for(l=0;l<8;l++){
                    data_ZigZag[i][j].Y[z[k*8+l]]=data_QUAN[i][j].Y[k][l];
                    data_ZigZag[i][j].Cb[z[k*8+l]]=data_QUAN[i][j].Cb[k][l];
                    data_ZigZag[i][j].Cr[z[k*8+l]]=data_QUAN[i][j].Cr[k][l];
                }
            }
        }
    }
}
```

#### Print data_ZigZag[0][0].Y for example

```js
for (k = 0; k < 8; k++) {
  for (l = 0; l < 8; l++) {
    printf("%3d ", data_ZigZag[0][0].Y[k * 8 + l]);
  }
}
```

![image](https://user-images.githubusercontent.com/128220508/226195195-68910b78-8b93-4ba7-81d4-8b5cf109d599.png)

### 6. DPCM

- Diff = DC(i)–DC(i-1)

![image](https://user-images.githubusercontent.com/128220508/226195570-ff3481ad-ea3c-40cd-be09-40b1d64adc83.png)

```js
void DPCM(ImgQUAN **data_QUAN, int H, int W){
    //先做第一行以外的
    for(i=0;i<H;i++){
        for(j=W;j>0;j--){
            data_QUAN[i][j].Y[0][0]=data_QUAN[i][j].Y[0][0]-data_QUAN[i][j-1].Y[0][0];
            data_QUAN[i][j].Cb[0][0]=data_QUAN[i][j].Cb[0][0]-data_QUAN[i][j-1].Cb[0][0];
            data_QUAN[i][j].Cr[0][0]=data_QUAN[i][j].Cr[0][0]-data_QUAN[i][j-1].Cr[0][0];
        }
    }
    //第一行的另外做 要扣掉上一行的
    for(i=H-1;i>0;i--){
        data_QUAN[i][0].Y[0][0]=data_QUAN[i][0].Y[0][0]-data_QUAN[i-1][W-1].Y[0][0];
        data_QUAN[i][0].Cb[0][0]=data_QUAN[i][0].Cb[0][0]-data_QUAN[i-1][W-1].Cb[0][0];
        data_QUAN[i][0].Cr[0][0]=data_QUAN[i][0].Cr[0][0]-data_QUAN[i-1][W-1].Cr[0][0];
    }
}
```

#### Print Y[0][0] in data_QUAN for example

```js
for (k = 0; k < 1; k++) {
  for (l = 0; l < 64; l++) {
    printf("%3d ", data_QUAN[k][l].Y[0][0]);
    if (l % 8 == 7) printf("\n");
  }
}
```

![image](https://user-images.githubusercontent.com/128220508/226196199-c4a51e73-0a29-4ebb-b12d-58ca43f4688d.png)

### 7. RLE

```js
void RLE(ImgRLE **data_RLE, ImgZigZag ** data_ZigZag, int H, int W){
    int Y_zero, Cb_zero, Cr_zero;
    int n1,n2,n3;
    for(i=0;i<H;i++){
        for(j=0;j<W;j++){
            Y_zero = 0;//計算連續幾個0
            Cb_zero = 0;
            Cr_zero = 0;
            n1=1;
            n2=1;
            n3=1;
            for(k=1;k<64;k++){
                //Y
                //值不為0時
                if((data_ZigZag[i][j].Y[k])!=0){
                    data_RLE[i][j].Y_value[n1]=data_ZigZag[i][j].Y[k];
                    data_RLE[i][j].Y_zero_num[n1]=Y_zero;
                    Y_zero=0;
                    n1++;
                }
                //值為0時Y_zero+1，計算連續多少個0
                else{ //data_ZigZag[i][j].Y[k]==0
                    Y_zero++;
                }

                //Cb
                if((data_ZigZag[i][j].Cb[k])!=0){
                    data_RLE[i][j].Cb_value[n2]=data_ZigZag[i][j].Cb[k];
                    data_RLE[i][j].Cb_zero_num[n2]=Cb_zero;
                    Cb_zero=0;
                    n2++;
                }
                else{
                    Cb_zero++;
                }

                //Cr
                if((data_ZigZag[i][j].Cr[k])!=0){
                    data_RLE[i][j].Cr_value[n3]=data_ZigZag[i][j].Cr[k];
                    data_RLE[i][j].Cr_zero_num[n3]=Cr_zero;
                    Cr_zero=0;
                    n3++;
                }
                else{
                    Cr_zero++;
                }
            }
            //最後都是0寫(0,0)
            data_RLE[i][j].Y_value[n1]=0;
            data_RLE[i][j].Y_zero_num[n1]=0;
            data_RLE[i][j].Cb_value[n2]=0;
            data_RLE[i][j].Cb_zero_num[n2]=0;
            data_RLE[i][j].Cr_value[n3]=0;
            data_RLE[i][j].Cr_zero_num[n3]=0;
        }
    }
}
```

#### Print data_RLE[0][0].Y for example

```js
for (i = 1; i < 15; i++) {
  printf("%3d ", data_RLE[0][0].Y_zero_num[i]);
  printf("%3d ", data_RLE[0][0].Y_value[i]);
}
```

![image](https://user-images.githubusercontent.com/128220508/226196263-e6e89bf0-65c4-437f-9a81-ea2b1f267a45.png)
