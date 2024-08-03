#pragma once

// AudioCapturer.h
#include <boost/asio.hpp> // boost::asio::detail::buffered_stream_storage::byte_type

#include <iostream>
#include <windows.h>	// WAVEFORMATEX & HWAVEOUT
#include <functional>	// std::function
#include <thread>		// threading and sleep
#include <chrono>		// high_resolution_clock
#include <vector>		// std::vector
#include <memory>		// waveHeaders
#include <mmeapi.h>		// WAVEFORMATEX / WAVEHDR / HWAVEIN / HWAVEOUT (redefine)
#include <string>		// std::to_string
#include "OpusEncoderMiddleware.h"
#include <ctime>
#include <iomanip>

#pragma comment(lib,"Winmm.lib") // Off LNK2019 for waveOutSetVolume, waveOutPrepareHeader, waveOutUnprepareHeader, waveOutWrite, waveOutReset

#define LOG(className, msg, type) (type == 0) ? std::cout << "[" #className << "] "  << msg << std::endl : std::cerr << "[" #className << "] Error: "  << msg << ". ErrCode/Datasize: " << type << std::endl;
#define DATA(className, msg, type) std::cout << "[" #className << "] "  << #msg << " is " << type << std::endl;

#define SOUND_DELAY_48000 5
#define DELAY_IN_SECONDS 1
#define BIT_PER_SAMPLE 16
#define SAMPLE_RATE 48000
#define SAMPLE_COUNT 5
#define NUM_BUFFERS 16
#define CHANNELS 1
#define SECONDS 2
#define NOISE 55

#define NUM_SAMPLES_8000 960
#define NUM_SAMPLES_16000 1920
#define NUM_SAMPLES_48000 5760

#define MAIN_SAMPLE_RATE 48000 

/*
*
* Audio capturer from microphone, version 1.3
* Powered    by    iWrench    at   31.07.2024
*
*/

class  AudioCapturer
{
public:

	/* Create instance */
	AudioCapturer();

	void Init(WAVEFORMATEX waveFormatEx);

	/* Get avaible microphone devices */
	std::vector<std::pair<int, std::string>> get_mic_devices();

	/* Get default microphone device */
	int GetDefaultMicrophoneDeviceIndex();

	/* Start capture from microphone */
	int StartAudioCapture();

	/* Stop capture from microphone */
	void StopAudioCapture();

	/* Pause capture from microphone */
	void PauseAudioCapture();

	/* Release capture from microphone */
	void ReleaseAudioCapture();

	std::function<void(std::vector<boost::asio::detail::buffered_stream_storage::byte_type> mdata, size_t dataSize, std::string timestamp)>onReceiveCallbackData;
	std::function<void(std::vector<boost::asio::detail::buffered_stream_storage::byte_type> mdata, size_t dataSize, std::string timestamp)>onReceiveCallbackCodec;

	std::function<size_t(signed short* aData, size_t dataSize, unsigned char * cbits)>proccessCodec;

	/* Get gain in last sample */
	int getGain();

	/* Returns the current audio stream encoding settings */
	WAVEFORMATEX getCurrentSettings();

	/* Return True if all microphone buffers not used */
	bool IsDeviceBusy();

	int noise = NOISE;
	int block_size = NUM_SAMPLES_48000;
	
	std::string getCurrentTimestamp();

	size_t max_data_bytes = 512;

private:

	WAVEFORMATEX waveFormat{};
	WAVEHDR waveHeader = { nullptr, 0, 0, 1 };
	HWAVEIN m_hWaveIn = nullptr;

	std::unique_ptr<WAVEHDR[]> waveHeaders;	// Storages for microphone audio data

	static void CALLBACK WaveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2); // Handler when microphone active

	int currentGain = 0;			// Current gain in last sample
	bool activeFlag = false;		// On/off WaveInProc handler and call onReceiveCallback

	int calculateSignalLevel(const signed char* audioData, int audioDataSize);	// Calculate gain in audioData
	
	// Default encoder
	OpusEncoderMiddleware oEncoder;

};

/*

How to use :

	// Default settings 
	WAVEFORMATEX waveFormat = {};
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nChannels = 1;
	waveFormat.nSamplesPerSec = MAIN_SAMPLE_RATE;
	waveFormat.wBitsPerSample = 16;
	waveFormat.nBlockAlign = waveFormat.nChannels * waveFormat.wBitsPerSample / 8;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	// Microphone capturer instance
	std::shared_ptr<AudioCapturer> aCapturer = std::make_shared<AudioCapturer>();

	// Set up custom settings to capturer 
	aCapturer->Init(waveFormat);

	aCapturer->onReceiveCallbackCodec = [&](std::vector<boost::asio::detail::buffered_stream_storage::byte_type> mdata, int dataSize, std::string timestamp) -> size_t {
	// Do something	
		
	};

	// Run listening microphone 
	std::thread setupThread(&AudioCapturer::StartAudioCapture, aCapturer.get());
	setupThread.join();


*/