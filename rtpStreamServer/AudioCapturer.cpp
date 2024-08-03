// AudioCapture.сpp

#include "AudioCapturer.h"

AudioCapturer::AudioCapturer()
{
    setlocale(LC_ALL, "Russian");
    LOG(AudioCapturer, "Create instance", 0)

    // Set defaults, if `Init` will not runned
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nChannels = CHANNELS;
    waveFormat.nSamplesPerSec = SAMPLE_RATE;
    waveFormat.wBitsPerSample = BIT_PER_SAMPLE;
    waveFormat.nBlockAlign = waveFormat.nChannels * waveFormat.wBitsPerSample / 8;
    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
    waveFormat.cbSize = 0;
}

void AudioCapturer::Init(WAVEFORMATEX _waveFormatEx)
{
    memcpy(&waveFormat, &_waveFormatEx, sizeof(WAVEFORMATEX));

    int blockSize = (waveFormat.nSamplesPerSec == 8000) ? NUM_SAMPLES_8000 : (waveFormat.nSamplesPerSec == 16000) ? NUM_SAMPLES_16000 : NUM_SAMPLES_48000;
    int frameSize = (blockSize > INT_MAX) ? INT_MAX : (int)blockSize;
    max_data_bytes = frameSize * waveFormat.nSamplesPerSec * waveFormat.nChannels * sizeof(signed short) / (1000 / 8);
}

std::vector<std::pair<int, std::string>> AudioCapturer::get_mic_devices()
{

    auto ConvertWcharToChar = [](const WCHAR* wstr) {
        int charCount = WideCharToMultiByte(CP_ACP, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
        char* result = new char[charCount];
        WideCharToMultiByte(CP_ACP, 0, wstr, -1, result, charCount, nullptr, nullptr);

        return result;
        };

    std::vector<std::pair<int, std::string>> deviceInfoList;

    UINT numDevs = waveInGetNumDevs();
    std::vector<WAVEINCAPS> deviceList(numDevs);
    for (UINT i = 0; i < numDevs; i++) {
        if (waveInGetDevCaps(i, &deviceList[i], sizeof(WAVEINCAPS)) == MMSYSERR_NOERROR) {
            std::string deviceName = ConvertWcharToChar(deviceList[i].szPname);
            deviceInfoList.push_back(std::make_pair(i, deviceName));
        }
    }

    return deviceInfoList;
}

int AudioCapturer::GetDefaultMicrophoneDeviceIndex()
{
    LOG(AudioCapturer, "Avaible devices: ", 0);
    std::vector<std::pair<int, std::string>> devices = get_mic_devices();

    if (devices.size() < 1) {
        std::cerr << "FATAL! No microphone devices available on this computer. Program terminated!" << std::endl;
        exit(1);
    }
    for (const auto& device : devices) {
        std::cout << "Device " << device.first << ": " << device.second << std::endl;
    }

    int defaultDeviceIndex = -1;
    int deviceCount = waveInGetNumDevs();

    for (int i = 0; i < deviceCount; i++)
    {
        WAVEINCAPS waveInCaps;
        if (waveInGetDevCaps(i, &waveInCaps, sizeof(WAVEINCAPS)) == MMSYSERR_NOERROR)
        {
            if (waveInCaps.dwFormats != 0)
            {
                defaultDeviceIndex = i;
                break;
            }
        }
    }

    LOG(AudioCapturer, "Default device: " + std::to_string(defaultDeviceIndex), 0)

        return defaultDeviceIndex;
}

int AudioCapturer::StartAudioCapture()
{
    LOG(AudioCapturer, "StartAudioCapture now...", 0)
        int selectedDeviceIndex = GetDefaultMicrophoneDeviceIndex();

    LOG(AudioCapturer, "WaveInOpen", waveInOpen(&m_hWaveIn, selectedDeviceIndex, &waveFormat, reinterpret_cast<DWORD_PTR>(WaveInProc), reinterpret_cast<DWORD_PTR>(this), CALLBACK_FUNCTION));

    block_size = (waveFormat.nSamplesPerSec == 8000) ? NUM_SAMPLES_8000 : (waveFormat.nSamplesPerSec == 16000) ? NUM_SAMPLES_16000 : NUM_SAMPLES_48000;

    waveHeaders.reset(new WAVEHDR[NUM_BUFFERS]);
    for (int i = 0; i < NUM_BUFFERS; i++) {
        waveHeaders[i].lpData = new char[block_size];
        waveHeaders[i].dwBufferLength = block_size;
        waveHeaders[i].dwUser = 0;
        waveHeaders[i].dwFlags = 0;
        waveHeaders[i].dwLoops = 0;

        LOG(AudioCapturer, "WaveInPrepareHeader", waveInPrepareHeader(m_hWaveIn, &waveHeaders[i], sizeof(WAVEHDR)));
        LOG(AudioCapturer, "WaveInAddBuffer", waveInAddBuffer(m_hWaveIn, &waveHeaders[i], sizeof(WAVEHDR)));

        // Set OPUS codec as default
        oEncoder.Init(waveFormat);

        proccessCodec = [&](signed short* aData, size_t dataSize, unsigned char* cbits) -> size_t {
            size_t size = oEncoder.Encode(aData, dataSize, cbits, max_data_bytes);
            return size;
        };

    }

    MMRESULT result = waveInStart(m_hWaveIn);

    if (result != MMSYSERR_NOERROR)
    {
        LOG(AudioCapturer, "StartAudioCapture error", result)
            return result;
    }
    else {
        LOG(AudioCapturer, "waveInStart", result);
    }

    ReleaseAudioCapture();

    LOG(AudioCapturer, "StartAudioCapture done... ", result);

    return MMSYSERR_NOERROR;
}

void AudioCapturer::StopAudioCapture()
{
    try {
        MMRESULT resultReset = waveInReset(m_hWaveIn);
        if (resultReset != MMSYSERR_NOERROR) {
            LOG(AudioCapturer, "waveInReset ", resultReset);
        }

        if (waveHeaders) {
            for (size_t i = 0; i < NUM_BUFFERS; ++i) {
                waveInUnprepareHeader(m_hWaveIn, &waveHeaders[i], sizeof(WAVEHDR));
                delete[] waveHeaders[i].lpData;
            }
        }
        waveInClose(m_hWaveIn);

    }
    catch (...) {
        LOG(AudioCapturer, "StopAudioCapture waveInReset / waveInUnprepareHeader / waveInClose", -1);
    }

    PauseAudioCapture();
}

void AudioCapturer::PauseAudioCapture()
{
    LOG(AudioCapturer, "PauseAudioCapture", 0)
        activeFlag = false;
}

void AudioCapturer::ReleaseAudioCapture()
{
    LOG(AudioCapturer, "ReleaseAudioCapture", 0)
        activeFlag = true;
}

std::string AudioCapturer::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();

    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm;
    localtime_s(&now_tm, &now_time);

    std::ostringstream oss;
    oss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();  // Добавляем миллисекунды
    return oss.str();
}


void AudioCapturer::WaveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    AudioCapturer* instance = reinterpret_cast<AudioCapturer*>(dwInstance);
    std::string currTimestamp = instance->getCurrentTimestamp();
    WAVEHDR* lpData = reinterpret_cast<WAVEHDR*>(dwParam1);

    if (instance->activeFlag) {
        if (lpData)
        {
            // This block wil be activated when lpData found
            if (lpData->dwBytesRecorded > 0) {

                int audioDataSize = lpData->dwBytesRecorded;

                signed char* audioData = reinterpret_cast<signed char*>(lpData->lpData);

                waveInAddBuffer(hwi, lpData, sizeof(WAVEHDR));

                std::thread sendThread([instance, audioData, audioDataSize, currTimestamp]() {
                    if (instance->calculateSignalLevel(audioData, audioDataSize) == PROGRESS_CONTINUE) {
                        // Send ordinary data
                        if (instance->onReceiveCallbackData) {
                            std::vector<boost::asio::detail::buffered_stream_storage::byte_type> mData;
                            mData.clear();
                            mData.assign(audioData, audioData + audioDataSize);
                            instance->onReceiveCallbackData(mData, audioDataSize, currTimestamp);
                        }

                        // send encoded data
                        if (instance->onReceiveCallbackCodec) {
                            size_t dataSize = (audioDataSize != instance->block_size * sizeof(signed short)) ? audioDataSize : instance->block_size * sizeof(signed short);

                            signed short* codecData = new signed short[dataSize];
                            memcpy(codecData, reinterpret_cast<signed short*>(audioData), dataSize);
                            if (instance->proccessCodec) {
                                const size_t mdb = instance->max_data_bytes;
                                std::vector<unsigned char> cbits(mdb);
                                ZeroMemory(cbits.data(), mdb);
                                size_t encodedBytes = instance->proccessCodec(codecData, dataSize, cbits.data());
                                std::vector<boost::asio::detail::buffered_stream_storage::byte_type> mData(cbits.data(), cbits.data() + encodedBytes);
                                instance->onReceiveCallbackCodec(mData, encodedBytes, currTimestamp);
                            }
                            else {
                                // Null codec connected - transit audio data
                                unsigned char* myAdata = reinterpret_cast<unsigned char*>(audioData);
                                std::vector<boost::asio::detail::buffered_stream_storage::byte_type> mData(myAdata, myAdata + dataSize);
                                instance->onReceiveCallbackCodec(mData, dataSize, currTimestamp);
                            }
                            delete[] codecData;
                        }
                    }

                    });
                sendThread.detach();
            }
        }
    }
    else {
        // This block wil be activated when no lpData found
        waveInAddBuffer(hwi, lpData, sizeof(WAVEHDR));
    }
}

int AudioCapturer::calculateSignalLevel(const signed char* audioData, int audioDataSize)
{
    double averageAmplitude = 0;
    for (int i = 0; i < audioDataSize / 2; i++) {
        short sample = audioData[i * 2] | (audioData[i * 2 + 1] << 8);
        averageAmplitude += abs(sample);
    }
    averageAmplitude /= static_cast<double>(audioDataSize / 2);

    currentGain = static_cast<unsigned short int>(averageAmplitude);

    if (currentGain > noise) {
        DATA(calculateSignalLevel,Current_gain, currentGain)
        return PROGRESS_CONTINUE;
    }
    else {
        return PROGRESS_QUIET;
    }
}

int AudioCapturer::getGain()
{
    return currentGain;
}

WAVEFORMATEX AudioCapturer::getCurrentSettings()
{
    return waveFormat;
}

bool AudioCapturer::IsDeviceBusy() {
    return activeFlag;
}
