#include "OpusEncoderMiddleware.h"

void OpusEncoderMiddleware::Init(WAVEFORMATEX _waveFormatEx)
{
    memcpy(&waveFormatEx, &_waveFormatEx, sizeof(WAVEFORMATEX));
    int error;
    encoder = opus_encoder_create(waveFormatEx.nSamplesPerSec, waveFormatEx.nChannels, OPUS_APPLICATION_VOIP, &error);

    blockSize = (waveFormatEx.nSamplesPerSec == 8000) ? NUM_SAMPLES_8000 : (waveFormatEx.nSamplesPerSec == 16000) ? NUM_SAMPLES_16000 : NUM_SAMPLES_48000;
    int frameSize = (blockSize > INT_MAX) ? INT_MAX : (int)blockSize;
    max_data_bytes = frameSize * waveFormatEx.nSamplesPerSec * waveFormatEx.nChannels * sizeof(signed short) / (1000 / 8);
}

OpusEncoderMiddleware::OpusEncoderMiddleware() : encoder(nullptr)
{
    // Do nothing
}

OpusEncoderMiddleware::~OpusEncoderMiddleware()
{
    opus_encoder_destroy(encoder);
}

int OpusEncoderMiddleware::Encode(const opus_int16* input, size_t inputSize, unsigned char* output, size_t _outputSize)
{
    int frameSize = (inputSize > INT_MAX) ? INT_MAX : (int)inputSize;
    int outputSize = (_outputSize > INT_MAX) ? INT_MAX : (int)_outputSize;
    return opus_encode(encoder, input, frameSize, output, outputSize);
}

