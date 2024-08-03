#pragma once
// rtpStreamServer.h

#include "AudioCapturer.h"
#include "aTcpClient.h"
#include <fstream>

#define HOST "127.0.0.1"
#define PORT "37777"

#define NUM_SAMPLES_8000 960
#define NUM_SAMPLES_16000 1920
#define NUM_SAMPLES_48000 5760

class rtpStreamServer {
public:
	static rtpStreamServer& getInstance();

	// Type: control
	// Start audio capture with standart keepAliveInterval as 10 sec
	bool start();
	// Type: control
	// Start audio capture with redeclared keepAliveInterval
	bool start(int keepAliveInterval);
	
	/* aCapturer API */
	
	// Getters
	
	// Type: Getter
	// Get current sound gain (this is not a volume)
	int getGainNow();
	// Type: Getter
	// Avaible microphone devices in system 
	std::vector<std::pair<int, std::string>> getDevices();
	// Type: Getter
	// Current audio settings for encoders and decoders
	WAVEFORMATEX getCurrentSettings();
	// Type: Getter
	// Calculated size of one audio block 120 ms in bytes
	size_t getBlockSize();
	// Type: Getter
	// Maximum capasibility of array for encoded data
	size_t getMaxEncodedAudioDataLen();

	// Type: Control
	// Pause audio capture
	void PauseAudioCapture();
	// Type: Control
	// Release  audio capture
	void ReleaseAudioCapture();
	
	// Type: Event
	// Runned when audio data prepared successufully
	std::function<void(std::vector<boost::asio::detail::buffered_stream_storage::byte_type> mdata, size_t dataSize, std::string timestamp)> OnData();
	// Type: Event
	// Runned when encoded audio data prepared successufully
	std::function<void(std::vector<boost::asio::detail::buffered_stream_storage::byte_type> mdata, size_t dataSize, std::string timestamp)> OnCodecData();
	
	// Type: Connector
	// Connector for codec middlewares, OPUS for example
	// NOTE: Codec OPUS included and configured as default !
	std::function<size_t(signed short* aData, size_t dataSize, unsigned char* cbits)> CodecConnector();

	/* aClientAPI */
	// Type: Validator
	// Check connection status now
	bool isConnect();
	// Type: Descriptor
	// Configured socket for use in another code. For receiving data, for example.
	boost::asio::ip::tcp::socket& getTcpSoketInstance();

	// Type: Setter
	// Set socket, who configured in another code. 
	// It may be use for sending data in same connection of another code
	void setTcpSocketInstance(boost::asio::ip::tcp::socket& socket);

	// Type: Setter
	// Set custom audio parameters
	// Note: Default settings is 1 channel, 16 bit and sample rate 48000Hz
	void setCurrentSettings(WAVEFORMATEX settings);

	// Type: control
	// Stop audio capture
	void stop();

	// Type:Setter 
	// Set endpoint for network connection
	void initRtpStreamServer(char* host, char* port);

	// Type:Setter 
	// Set endpoint for network connection used program parameters
	void initRtpStreamServer(int argc, char* argv[]);

private:

	rtpStreamServer();

	const char* host = HOST;
	const char* port = PORT;
	boost::asio::io_service io_service;
	std::unique_ptr<aTcpClient> tClient;
	std::shared_ptr<AudioCapturer> aCapturer;

	// aCapturer variables
	// Calculated size of one audio block 120 ms in bytes
	int blockSize;
	// Maximum capasibility of array for encoded data
	size_t maxEncodedAudioDataLen;
};

/*

	how to use

	rtpStreamServer& serverInstance = rtpStreamServer::getInstance();

    char host[] = "example.com";
    char port[] = "8080";
    serverInstance.initRtpStreamServer(host, port);

    bool result = serverInstance.start();

    (result) ? std::cout << "Test sucessufully " << std::endl : std::cerr << "Test failed - could not connect " << std::endl;


*/