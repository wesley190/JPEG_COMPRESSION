#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <time.h>

struct wavfile_header //設定wav檔格式
{
    char riff_tag[4];
    int riff_length;
    char wave_tag[4];
    char fmt_tag[4];
    int fmt_length;
    short audio_format;
    short num_channels;
    int sample_rate;
    int byte_rate;
    short block_align;
    short bits_per_sample;
    char data_tag[4];
    int data_length;
};
int i,n,k;

void gen_cos(int fs, int f,char* filename){
	int T = 1;
	int SampleBit=16;
    int A=10000;

    struct wavfile_header header;
	strncpy(header.riff_tag, "RIFF", 4); //RIFF識別標誌
    strncpy(header.wave_tag, "WAVE", 4); //WAVE識別標誌
    strncpy(header.fmt_tag, "fmt ", 4);  //fmt識別標誌
    strncpy(header.data_tag, "data", 4); //資料標記符data

    header.riff_length = fs*(SampleBit/8)*T+36;//wav格式檔案長度
    header.fmt_length = 16;                    //過度位元組
    header.audio_format = 1;                   //格式類別
    header.num_channels = 1;                   //單聲道，設定1
    header.sample_rate = fs;                 //取樣率
    header.byte_rate = fs * (SampleBit / 8); //音頻傳送速率
    header.block_align = SampleBit / 8;        //一個樣本多聲道資料塊大小
    header.bits_per_sample = SampleBit;        //一樣本包含bit數
    header.data_length = fs*(SampleBit/8)*T;   //音頻長度 
	
	FILE *file;
	file = fopen(filename,"wb+");			//以二進位制寫入的方式打開檔案
	fwrite(&header, sizeof(header),1, file);//將header寫入檔案
	
	short cosine;
	for( i=0 ; i < fs*T ; i++ ){
		cosine = floor(A*cos(2*M_PI*f*i/fs)+0.5);//生成cosine
		fwrite(&cosine,sizeof(short),1,file);
	}
	fclose(file);
};
void DFT(float* X,int num,int* add,int* multi,char* filename,FILE* fp){
	float Real=0,Image=0;//將e^(jwt)拆成cos(wt)-i*sin(wt)
	float Xn;
	
		for(k=0;k<num;k++){
			Real=0;
			Image=0;
			for(n=0;n<num;n++){
				Real += cos((2*M_PI/num)*n*k)*X[n];//X[n]*e^(jwt) = X[n]*(cos(wt)-i*sin(wt))
				Image -= sin((2*M_PI/num)*n*k)*X[n];	
				*add+=2;
				*multi+=2;
			}
			Xn =20*log10(sqrt(Real * Real + Image * Image));
		
			*multi+=1;
			fprintf( fp, "%f ", Xn);
		}
}
void gen_spectrogram(int A_WT,double A_WS,double DFT_WS,double FrameInterval,int fs,char filename[],char txtname[],int T){
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
		fread(wave,2,fs*T+22,wav);//
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

	float Xn;
	float *X = calloc(N,sizeof(float));
	int add =0,multiple =0;
	
	FILE *txt;
	txt = fopen(txtname,"w+");

	for(i=0;i<m;i++){
		for(k=0;k<N;k++){
			X[k] = value[i*M+k]*w[k];	//X[n] = x[m*M+n]*w[n]
			multiple += 1;
		}
		
		DFT(X,N,&add,&multiple,filename,txt);
		fprintf( txt, "\n" );
	}
	
	printf("%s   1.addition: %d   2.multiplication: %d\n",txtname,add,multiple);
	
	free(value);
	free(w);
	free(X);

	fclose(txt);
} 

int main(int argc, char *argv[]){
	gen_cos(16000,50,"cos_050Hz-16k.wav");
	gen_cos(16000,220,"cos_220Hz-16k.wav");
	
	gen_cos(8000,50,"cos_050Hz-8k.wav");
	gen_cos(8000,220,"cos_220Hz-8k.wav");

	//設定並輸出spectrogram of Set1
	gen_spectrogram(0, 0.005, 0.008, 0.005, 8000, "cos_050Hz-8k.wav", "cos_050Hz-8k.{Set1}.txt",1);
	gen_spectrogram(0, 0.005, 0.008, 0.005, 8000, "cos_220Hz-8k.wav", "cos_220Hz-8k.{Set1}.txt",1);
	gen_spectrogram(0, 0.005, 0.008, 0.005, 8000, "vowel-8k.wav", "vowel-8k.{Set1}.txt",5);

	gen_spectrogram(0, 0.005, 0.008, 0.005, 16000, "cos_050Hz-16k.wav", "cos_050Hz-16k.{Set1}.txt",1);
	gen_spectrogram(0, 0.005, 0.008, 0.005, 16000, "cos_220Hz-16k.wav", "cos_220Hz-16k.{Set1}.txt",1);
	gen_spectrogram(0, 0.005, 0.008, 0.005, 16000, "vowel-16k.wav", "vowel-16k.{Set1}.txt",5);

	//設定並輸出spectrogram of Set2
	gen_spectrogram(1, 0.02, 0.032, 0.01, 8000, "cos_050Hz-8k.wav", "cos_050Hz-8k.{Set2}.txt",1);
	gen_spectrogram(1, 0.02, 0.032, 0.01, 8000, "cos_220Hz-8k.wav", "cos_220Hz-8k.{Set2}.txt",1);
	gen_spectrogram(1, 0.02, 0.032, 0.01, 8000, "vowel-8k.wav", "vowel-8k.{Set2}.txt",5);

	gen_spectrogram(1, 0.02, 0.032, 0.01, 16000, "cos_050Hz-16k.wav", "cos_050Hz-16k.{Set2}.txt",1);
	gen_spectrogram(1, 0.02, 0.032, 0.01, 16000, "cos_220Hz-16k.wav", "cos_220Hz-16k.{Set2}.txt",1);
	gen_spectrogram(1, 0.02, 0.032, 0.01, 16000, "vowel-16k.wav", "vowel-16k.{Set2}.txt",5);
	
	

}
