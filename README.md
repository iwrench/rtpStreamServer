# rtpStreamServer
Готовый rtpStreamServer v1.0 для проекта Мяудза.
Подготовлен по заказу ООО "Уникальный код"

## Назначение

Используется для приема звука с микрофона и отправки его в сеть на определенный адрес пакетами TCP
Есть API для подключения стороннего кодека (по умолчанию подключен OPUS/48000) и управления (стоп, 
старт, пауза, возобновление, проверка соединения, проверка текущего уровня звука).

**Внимание: ** Класс инкапсюлирован и поставляется как есть. Все ошибки обработаны, произведено тестирование на 
возникающие ошибки в ходе выполнения. Все выявленые ошибки устранены.

## Зависимости:

###  boost/asio
Устанавливается через менеждер пакетов разработчика vcpkg ( Документация и ссылка на гитхаб тут https://vcpkg.io/en/)
```
.\vcpkg install boost-asio
.\vcpkg integrate install
```

### opus
Устанавливается через менеждер пакетов разработчика vcpkg ( Документация и ссылка на гитхаб тут https://vcpkg.io/en/)
```
.\vcpkg install opus
.\vcpkg integrate install
```

## Пример использования
```
	#include "rtpStreamServer.h"
	#include <conio.h>
	
	int main(int argc, char* argv[])
	{
	    // Get singleton instance
	    rtpStreamServer& serverInstance = rtpStreamServer::getInstance();
	
	    // Connect to SFU server
	    char host[] = "example.com";
	    char port[] = "8080";
	    serverInstance.initRtpStreamServer(host, port);
	    
	    // Add custom codec (alaw,ulaw,g722 etc...)
	    serverInstance.CodecConnector() = [&](signed short* aData, size_t dataSize, unsigned char* cbits) -> size_t {
	        // Some codec code
	        // for example something like this:
	        
	        size_t max_data_bytes = 512;
	        OpusEncoderMiddleware oEncoder;
	        size_t size = oEncoder.Encode(aData, dataSize, cbits, max_data_bytes);
	        return size;
	    };
	
	    // Run audio capture system
	    bool result = serverInstance.start();
	
	    // Print test status
	    (result) ? std::cout << "Test sucessufully " << std::endl : std::cerr << "Test failed - could not connect " << std::endl;
	
	    // Program terminated only when pressed ESC.
	    // NOTE: rtpStreamServer don't include this mechanizm
	    char ch;
	    std::cout << "Press ESC for terminate...\n";
	    do {
	        ch = _getch();
	    } while (ch != 27); // 27 - это ASCII-код для ESC
	
	    std::cout << "Bye!\n";
	    return 0;
	};
```

# Описание дочерних классов

## Класс AudioCapturer
Прием данных с микрофона, имеет API для подключения кодека и возможность выбрать качество 
передаваемых данных ( частоту дискретизации) перед использованием (не runtime опция).

Зависит от  
### mmsystem
это предустановленная библиотека в windows 10 / 11, отвечающий за внешее аудио оборудование
### aTcpClient
этот класс поставляется в настоящем пакете, отвечает за сетевой траффик (отсылка и прием)

### Пример использования 

```
	WAVEFORMATEX waveFormat = {};
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nChannels = 1;
	waveFormat.nSamplesPerSec = MAIN_SAMPLE_RATE;
	waveFormat.wBitsPerSample = 16;
	waveFormat.nBlockAlign = waveFormat.nChannels * waveFormat.wBitsPerSample / 8;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * 	waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	/* Microphone capturer instance */
	std::shared_ptr<AudioCapturer> aCapturer = std::make_shared<AudioCapturer>();
	
	/* Set up custom settings to capturer */
	aCapturer->Init(waveFormat);
	
	/* Set up callback for complete audio blocks when it is be ready */
	aCapturer->onReceiveCallbackCodec = [&](std::vector<boost::asio::detail::buffered_stream_storage::byte_type> mdata, size_t dataSize, std::string timestamp) -> size_t {
	
	//std::cout << "Запущен обработчик для " << mdata.size() << " байт" << std::endl;
	size_t sended = 0;
	if (tClient) {
	if (tClient->isConnect()) {
	    sended = tClient->sendData(mdata);
	}
	else {
	    sended = -1;
	}
	if (sended == -1) {
	    LOG(tClient, "Stoping audio net communicator (tClient)", 0);
	    aCapturer->StopAudioCapture();
	    tClient->stop();
	    LOG(tClient, "Stoped audio net communicator (tClient)", 0);
	}
	};
	return sended;
	};
	
	/* Run listening microphone */
	std::thread setupThread(&AudioCapturer::StartAudioCapture, aCapturer.get());
	setupThread.join();
```

## Класс OpusEncoderMiddleware
Кодирование аудиопотока в кодек OPUS

Зависимость от 
### opus 
Это стандартная библиотека, может быть поставлена через менеждер пакетов разработчика vcpkg ( Документация и ссылка на гитхаб тут https://vcpkg.io/en/)
```
.\vcpkg install opus
.\vcpkg integrate install
```
```
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
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * 		waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;
	
	OpusEncoderMiddleware oEncoder;
	oEncoder.Init(waveFormat);
	
	opus_int16* opusData = new opus_int16[streamPlayer.block_size_16000];
	memcpy(opusData, reinterpret_cast<signed short*>(lpWaveBuf), streamPlayer.block_size_16000 * sizeof(opus_int16));
	unsigned char* cbits = new unsigned char[MAX_ENCODED_AUDIO_DATA_LEN];
	ZeroMemory(cbits, MAX_ENCODED_AUDIO_DATA_LEN);
	int size = oEncoder.Encode(opusData, streamPlayer.block_size_16000, cbits, 	MAX_ENCODED_AUDIO_DATA_LEN);
	LOG_ME(OpusEncoderMiddleware, "Encoded " + std::to_string(size) + " bytes...", 	0);
	delete[] opusData;
	// .. do something which encoded data in `cbits`, send over TCP, for example
	delete[] cbits;
	} // end of lpWaveBuf != NULL

    Output:
    [OpusEncoderMiddleware] Encoded 197 bytes.... ErrCode/Datasize: 0
```

## Класс aTcpClient
Обеспечивает отправку и прием TCP пакетов, а так же функцию обратной связи, срабатывающую по событию.

Зависимость от  
### boost/bind 
### boost/asio
### boost/thread 
Это стандартные библиотеки, могут быть поставлены через менеждер пакетов для разработчиков - vcpkg
```
.\vcpkg install boost-bind boost-asio boost-thread 
.\vcpkg integrate install
```
```
 boost::asio::io_service io_service;
    std::unique_ptr<aTcpClient> tClient = std::make_unique<aTcpClient>(io_service);
    tClient->connectToServer(host, port);
    tClient->setCheckInterval(10);
    std::thread checkThread(&aTcpClient::checkConnection, tClient.get());
    checkThread.detach();

    tClient->setOnReceiveCallback([&](const std::vector<unsigned char>& lpWaveFromTcp) 	{  some code… });
        std::thread recvThread([tClient = std::move(tClient), nSamplesPerSec = 	waveFormat.nSamplesPerSec]() {
        tClient->listenTcpData(nSamplesPerSec);
        });
    recvThread.join();
```
