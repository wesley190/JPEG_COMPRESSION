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

void wavfile_close(FILE *file) //此function用意為建立檔案後，可以得知正確音訊資料大小與檔案長度再輸出，確保wav檔格式正確
{
    int file_length = ftell(file); //ftell返回檔案目前位置

    int data_length = file_length - sizeof(struct wavfile_header);      //音訊資料長度 = 檔案長度-wav設定格式大小
    fseek(file, sizeof(struct wavfile_header) - sizeof(int), SEEK_SET); //fseek找到要填入音訊長度的位置
    fwrite(&data_length, sizeof(data_length), 1, file);                 //fwrite寫入音訊長度

    int riff_length = file_length - 8; //wav格式檔案長度 = 文件長度 - 8
    fseek(file, 4, SEEK_SET);
    fwrite(&riff_length, sizeof(riff_length), 1, file);

    fclose(file);
}

int main(int argc, char *argv[])
{
    int Rate = atoi(argv[1]); //atoi轉化char成int資料結構
    int SampleBit = atoi(argv[2]);
    int Freq = atoi(argv[3]);
    double M = atof(argv[4]);
    double T = atof(argv[5]);

    double Err; //代表quantization error/noise
    double X;   //代表量化前的訊號值
    double Q;   //代表量化後的值
    double SQNR;
    long double ErrSum = 0;  //代表quantization noise的 power
    long double DataSum = 0; //代表原本信號的power

    const int NUM_SAMPLES = Rate * T; //取樣個數
    int length = NUM_SAMPLES;

    struct wavfile_header header;

    strncpy(header.riff_tag, "RIFF", 4); //RIFF識別標誌
    strncpy(header.wave_tag, "WAVE", 4); //WAVE識別標誌
    strncpy(header.fmt_tag, "fmt ", 4);  //fmt識別標誌
    strncpy(header.data_tag, "data", 4); //資料標記符data

    header.riff_length = 0;
    header.fmt_length = 16;                    //過度位元組
    header.audio_format = 1;                   //格式類別
    header.num_channels = 1;                   //單聲道，設定1
    header.sample_rate = Rate;                 //取樣率
    header.byte_rate = Rate * (SampleBit / 8); //音頻傳送速率
    header.block_align = SampleBit / 8;        //一個樣本多聲道資料塊大小
    header.bits_per_sample = SampleBit;        //一樣本包含bit數
    header.data_length = 0;

    FILE *f = stdout;
    fflush(f);

    fwrite(&header, sizeof(header), 1, f); //wav格式輸出至stdout

    if (SampleBit == 8)
    {
        char waveform[NUM_SAMPLES]; //一樣本大小為1 byte，故設定為char
        for (int i = 0; i < length; i++)
        {
            double t = (double)i / Rate;
            int data = floor((128 * (M * sin(Freq * t * 2 * M_PI) + 1) + 0.5)); //將振幅設定[0,255]，並且量化
            if (data == 256)
                data = 255;
            waveform[i] = data;

            X = 128 * (M * sin(Freq * t * 2 * M_PI) + 1);
            Q = data;
            Err = Q - X; //quantization noise = 量化後訊號值-原本訊號值
            DataSum += pow(X, 2);
            ErrSum += pow(Err, 2);
        }

        fwrite(waveform, sizeof(char), length, f); //將樣本值輸出至stdout
        wavfile_close(f);
    }
    else if (SampleBit == 16)
    {
        short waveform[NUM_SAMPLES]; //一樣本大小為2 byte，故設定為short
        for (int i = 0; i < length; i++)
        {
            double t = (double)i / Rate;
            int data = floor((pow(2, 15) * M * sin(Freq * t * 2 * M_PI) + 0.5)); //將振幅設定[-2^15,2^15-1]，並且量化
            if (data == pow(2,15))
                data = pow(2, 15) - 1; //預防溢位
            waveform[i] = data;

            X = pow(2, 15) * M * sin(Freq * t * 2 * M_PI);
            Q = data;
            Err = Q - X;
            DataSum += pow(X, 2);
            ErrSum += pow(Err, 2);
        }

        fwrite(waveform, sizeof(short), length, f);
        wavfile_close(f);
    }
    else
    {
        int waveform[NUM_SAMPLES]; //一樣本大小為4 byte，故設定為int
        for (int i = 0; i < length; i++)
        {
            double t = (double)i / Rate;
            int data = floor((pow(2, 31) * M * sin(Freq * t * 2 * M_PI) +0.5)); //將振幅設定[-2^31,2^31-1]，並且量化
            if ((M*sin(Freq * t * 2 * M_PI)) == 1)
                data = pow(2, 31) - 1; //預防溢位
            waveform[i] = data;

            X = pow(2, 31) * M * sin(Freq * t * 2 * M_PI);
            Q = data;
            Err = Q - X;
            DataSum += pow(X, 2);
            ErrSum += pow(Err, 2);
        }

        fwrite(waveform, sizeof(int), length, f);
        wavfile_close(f);
    }

    DataSum = DataSum / NUM_SAMPLES;     //原本訊號的power
    ErrSum = ErrSum / NUM_SAMPLES;       //quantization noise 的power
    SQNR = 10 * log10(DataSum / ErrSum); //產生SQNR
    fprintf(stderr, "%.15f", SQNR);      //print SQNR至stderr

    return 0;
}