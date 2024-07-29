#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <time.h>

int i,n,k;

char* pitch(float freq){
    float a;
    int b;
    int temp;
    char* v = malloc(4);
    strcpy(v,"A4 ");

    a = freq/440;   //輸入頻率和A4校準
    if(a == 0){
        strcpy(v,"non");
        return v;
    }
    b = round(12*log2(a));  //因有誤差，用round()調整
    while(b > 2 || b <-9){  //同一個八度，b=-9時為C，b=2時為B
        if(b < -9){
            b += 12;
            temp = v[1] - '0';
            v[1] = (temp - 1) + '0';
        }
        else{
            b -= 12;
            temp = v[1] - '0';
            v[1] = (temp + 1) + '0';
        }
    }
    if(b == 0){
        v[0] = 'A';
    }
    else if(b == 1){
        v[0] = 'A';
        v[2] = '#';
    }
    else if(b == 2){
        v[0] = 'B';
    }
    else if(b == -9){
        v[0] = 'C';
    }
    else if(b == -8){
        v[0] = 'C';
        v[2] = '#';
    }
    else if(b == -7){
        v[0] = 'D';
    }
    else if(b == -6){
        v[0] = 'D';
        v[2] = '#';
    }
    else if(b == -5){
        v[0] = 'E';
    }
    else if(b == -4){
        v[0] = 'F';
    }
    else if(b == -3){
        v[0] = 'F';
        v[2] = '#';
    }
    else if(b == -2){
        v[0] = 'G';
    }
    else if(b == -1){
        v[0] = 'G';
        v[2] = '#';
    }

    return v;
}
void gen_spectrogram(int A_WT,double A_WS,double DFT_WS,double FrameInterval,int fs,char filename[],char txtname[],char labname[],int T){
	int N,P,m,M;
	P= A_WS*fs;				//P為做window func的樣本數
	N = DFT_WS*fs;			//做DFT的樣本數
	M = FrameInterval*fs;	//framing間隔樣本數
	m = (1/FrameInterval)*T;//Frame總數量

	FILE *wav;
	wav = fopen(filename,"rb+");//以二進位制讀取的方式打開檔案
	
	short *wave= calloc((fs*T +22),sizeof(short));//動態配置wave,value兩個陣列，其中wave因包含wav的header，故總大小加上44b
	short *value = calloc(fs*T,sizeof(short));
	
	if(wav == NULL){
		printf("Failure: open\n");
	}
	else{
		fread(wave,2,fs*T+22,wav);
	}
	fclose(wav);
	
	for(i=22;i<fs*T+22;i++){
	value[i-22] = wave[i];	//篩選出wav header之後的cosine值
	}
	free(wave);

	float *w;				//初始化window function w
	w =  calloc(N,sizeof(double));//calloc將w初始值預設為零

	if(A_WT==0){		//rectangular
		for(n=0;n<P;n++){
			w[n]=1;
		}
	}
	else if(A_WT==1){	//hamming
		for(n=0;n<P;n++){
			w[n]=0.54-0.46*cos(2*M_PI*n/(P-1));
		}
	}

	float Real=0,Image=0;//將e^(jwt)拆成cos(wt)-i*sin(wt)
    float Xn;
    float big = 0;
    float F;
    double t_s,t_e;
    int nu;
    char *v1 = malloc(4);
    char *test = malloc(4);

	float *X = calloc(N,sizeof(float));
	
	FILE *txt;
	txt = fopen(txtname,"w+");
    FILE *lab;
    lab = fopen(labname,"w+");

    strcpy(v1,"non");    //複製用法
    t_s = 0;

	for(i=0;i<m;i++){
        nu = 0;
        big = 0;
		for(k=0;k<N;k++){
			X[k] = value[i*M+k]*w[k];	//X[n] = x[m*M+n]*w[n]
		}
	
		for(k=0;k<N/2+1;k++){
			Real=0;
			Image=0;
			for(n=0;n<N;n++){
				Real += cos((2*M_PI/N)*n*k)*X[n];//X[n]*e^(jwt) = X[n]*(cos(wt)-i*sin(wt))
				Image -= sin((2*M_PI/N)*n*k)*X[n];	
			}
			Xn =20*log10(sqrt(Real * Real + Image * Image));
            if(Xn > big){
                big = Xn;   //找出最大強度
                nu = k;     //最大強度的行數
            }

			fprintf( txt, "%f ", Xn);
		}
        F = nu*fs/N;
        if(F>1500) F/=2;
        test = pitch(F);
        if(strcmp(test,v1) != 0){   //字串比較用法
            t_e = (double)(i*FrameInterval+2*(double)nu/(double)fs);  //test音高開始的時間
            fprintf(lab,"%.7f %.7f %s\n",t_s,t_e,v1);
            t_s = t_e;
            strncpy(v1,test,4);
        }

		fprintf( txt, "\n" );
	}
	
	
	free(value);
	free(w);
	free(X);
    free(test);
    free(v1);

	fclose(txt);
    fclose(lab);
} 

int main(int argc, char *argv[]){
    char* name = argv[1];
    char* labname = malloc(50);
    int T;

    if(name[0] == 'P'){
        strcpy(labname,"Prelude_No._1_in_C_Major_BWV_846_for_Flute.lab");
        T =  15;
    }
    else if(name[0] == 'B'){
        strcpy(labname,"Badinerie_for_flute_by_JS_Bach.lab");
        T = 16;
    }
    else{
        strcpy(labname,"FamilyMart-Right-Channel.lab");
        T = 8;
    }

	//設定並輸出spectrogram of Set2
	gen_spectrogram(1, 0.04, 0.04644, 0.04, 44100, name, "test1.txt",labname,T);
	
	
}
