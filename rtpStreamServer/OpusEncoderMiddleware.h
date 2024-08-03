#pragma once

/*
    OpusEncoderMiddleware v 3.0 reliz
    Created by iWrench at  18.07.2024
*/

#include  "opus/opus.h"     // OpusEncoder
#include <windows.h>        // WAVEFORMATEX
#include <iostream>         // std::cout and std::endl
#include <string>           // std::to_string

#define LOG_ME(className, msg, type) (type == 0) ? std::cout << "[" #className << "] "  << msg << std::endl : std::cerr << "[" #className << "] Error: "  << msg << ". ErrCode/Datasize: " << type << std::endl;

#ifndef OPUS_ENCODER_MIDDLEWARE_H
#define OPUS_ENCODER_MIDDLEWARE_H

#define NUM_SAMPLES_8000 960
#define NUM_SAMPLES_16000 1920
#define NUM_SAMPLES_48000 5760

class  OpusEncoderMiddleware {
private:
    OpusEncoder* encoder;
    WAVEFORMATEX waveFormatEx{};
public:
    OpusEncoderMiddleware();
    ~OpusEncoderMiddleware();

    size_t max_data_bytes = 512;
    size_t blockSize = NUM_SAMPLES_48000;

    /*
    * const opus_int16* input is const signed short*
    * const signed short* input = ...; // Your data
    * const opus_int16* opusInput = reinterpret_cast<const opus_int16*>(input);
    */

    int Encode(const opus_int16* input, size_t inputSize, unsigned char* output, size_t outputSize);
    void Init(WAVEFORMATEX waveFormatEx);
};

#endif

/*
    How to use:

    #define MAX_ENCODED_AUDIO_DATA_LEN 512
    #define BLOCK_SIZE_16000 1920
    // streamPlayer.block_size_16000 same as BLOCK_SIZE_16000
    LPVOID lpWaveBuf = (char*)lpWaveFromTcp;
    if (lpWaveBuf != NULL) {

        WAVEFORMATEX waveFormat = {};
        waveFormat.wFormatTag = WAVE_FORMAT_PCM;
        waveFormat.nChannels = 1;
        waveFormat.nSamplesPerSec = 16000;
        waveFormat.wBitsPerSample = 16;
        waveFormat.nBlockAlign = waveFormat.nChannels * waveFormat.wBitsPerSample / 8;
        waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
        waveFormat.cbSize = 0;

        OpusEncoderMiddleware oEncoder;
        oEncoder.Init(waveFormat);

        opus_int16* opusData = new opus_int16[streamPlayer.block_size_16000];
        memcpy(opusData, reinterpret_cast<signed short*>(lpWaveBuf), streamPlayer.block_size_16000 * sizeof(opus_int16));
        unsigned char* cbits = new unsigned char[MAX_ENCODED_AUDIO_DATA_LEN];
        ZeroMemory(cbits, MAX_ENCODED_AUDIO_DATA_LEN);
        int size = oEncoder.Encode(opusData, streamPlayer.block_size_16000, cbits, MAX_ENCODED_AUDIO_DATA_LEN);
        LOG_ME(OpusEncoderMiddleware, "Encoded " + std::to_string(size) + " bytes...", 0);
        delete[] opusData;
        // .. do something which encoded data in `cbits`, send over TCP, for example
        delete[] cbits;
    } // end of lpWaveBuf != NULL

    Output:
    [OpusEncoderMiddleware] Encoded 197 bytes.... ErrCode/Datasize: 0
*/