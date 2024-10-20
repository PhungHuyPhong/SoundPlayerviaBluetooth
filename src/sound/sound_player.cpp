#include<iostream>
#include<portaudio.h>
#include<mpg123.h>
#define BUFFER_SIZE 4096
using namespace std;

struct AudioData{
    unsigned char buffer[BUFFER_SIZE];
    size_t buffer_size;
    mpg123_handle* mpg123;
    PaStream* stream;
};
static int AudioCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData){
    AudioData* data = (AudioData*) userData;
    unsigned char* out = (unsigned char*) outputBuffer;
    size_t bytesRead;
    int err = mpg123_read(data->mpg123,data->buffer,BUFFER_SIZE, &bytesRead);
    if(err == MPG123_DONE) return paComplete;
    copy(data->buffer, data->buffer + bytesRead, out);
    return paContinue;
}
int main(int argc, char* argv[]){
    if(argc != 2){
        cerr<<"Usage: "<<argv[0]<<" <mp3_file>"<<endl;
        return 1;
    }
    mpg123_init();
    mpg123_handle* mpg123 = mpg123_new(NULL, NULL);
    if(mpg123_open(mpg123,argv[1]) != MPG123_OK){
        cerr<<"ERROR: Unable to open MP3 file!"<<endl;
        mpg123_delete(mpg123);
        return 1;
    }
    int channels, encoding;
    long rate;
    mpg123_getformat(mpg123,&rate,&channels,&encoding);
    Pa_Initialize();
    AudioData audioData = { {}, 0, mpg123, nullptr};
    Pa_OpenDefaultStream(&audioData.stream, 0, channels, paInt16, rate, BUFFER_SIZE / channels, AudioCallback, &audioData);
    Pa_StartStream(audioData.stream);
    cout<<"Playing MP3 file: "<<argv[1]<<endl;
    while(Pa_IsStreamActive(audioData.stream)){
        Pa_Sleep(100);
    }
    Pa_StopStream(audioData.stream);
    Pa_CloseStream(audioData.stream);
    Pa_Terminate();
    mpg123_close(mpg123);
    mpg123_delete(mpg123);
    mpg123_exit();
    cout<<"Playback finished!"<<endl;
    return 0;
}