#include "rtpStreamServer.h"
#include <conio.h> // gets

rtpStreamServer& rtpStreamServer::getInstance()
{
    static rtpStreamServer instance;
    return instance;
}

rtpStreamServer::rtpStreamServer() : tClient(std::make_unique<aTcpClient>(io_service)), aCapturer (std::make_shared<AudioCapturer>()), blockSize(NUM_SAMPLES_48000), maxEncodedAudioDataLen(512)
{
    setlocale(LC_ALL, "Russian");
    host =  HOST;
    port =  PORT;
}

void rtpStreamServer::initRtpStreamServer(char* _host, char* _port)
{
    setlocale(LC_ALL, "Russian");
    host = _host;
    port = _port;
}

void rtpStreamServer::initRtpStreamServer(int argc, char* argv[])
{
    setlocale(LC_ALL, "Russian");
    host = (argc > 1) ? argv[1] : HOST;
    port = (argc > 2) ? argv[2] : PORT;
}

bool rtpStreamServer::start() {
    return start(10);
}

bool rtpStreamServer::start(int keepAliveInterval)
{
    /* aTcpClient block */

    // Try to connect to server
    tClient->connectToServer(host, port);
    // Run keepAlive system
    tClient->setCheckInterval(10);
    std::thread checkThread(&aTcpClient::checkConnection, tClient.get());
    checkThread.detach();

    /* Capturing the flag... ups =) audio  */
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

    std::thread setupThread(&AudioCapturer::StartAudioCapture, aCapturer.get());
    setupThread.detach();

    blockSize = aCapturer->block_size;
    maxEncodedAudioDataLen = aCapturer->max_data_bytes;

    return tClient->isConnect();
}

int rtpStreamServer::getGainNow()
{
    return aCapturer->getGain();
}

std::vector<std::pair<int, std::string>> rtpStreamServer::getDevices()
{
    return aCapturer->get_mic_devices();
}

WAVEFORMATEX rtpStreamServer::getCurrentSettings()
{
    return aCapturer->getCurrentSettings();
}

size_t rtpStreamServer::getBlockSize()
{
    return (blockSize)? blockSize: NUM_SAMPLES_48000;
}

size_t rtpStreamServer::getMaxEncodedAudioDataLen()
{
    return (maxEncodedAudioDataLen) ? maxEncodedAudioDataLen : 512;
}

void rtpStreamServer::PauseAudioCapture()
{
    aCapturer->PauseAudioCapture();
}

void rtpStreamServer::ReleaseAudioCapture()
{
    aCapturer->ReleaseAudioCapture();
}

std::function<void(std::vector<boost::asio::detail::buffered_stream_storage::byte_type> mdata, size_t dataSize, std::string timestamp)> rtpStreamServer::OnData()
{
    return aCapturer->onReceiveCallbackData;
}

std::function<void(std::vector<boost::asio::detail::buffered_stream_storage::byte_type>mdata, size_t dataSize, std::string timestamp)> rtpStreamServer::OnCodecData()
{
    return aCapturer->onReceiveCallbackCodec;
}

std::function<size_t(signed short* aData, size_t dataSize, unsigned char* cbits)> rtpStreamServer::CodecConnector()
{
    return aCapturer->proccessCodec;
}

bool rtpStreamServer::isConnect()
{
    return tClient->isConnect();
}

boost::asio::ip::tcp::socket& rtpStreamServer::getTcpSoketInstance()
{
    return tClient->getSocket();
}

void rtpStreamServer::setTcpSocketInstance(boost::asio::ip::tcp::socket& socket) {
    tClient->setSocket(socket);
}

void rtpStreamServer::setCurrentSettings(WAVEFORMATEX settings)
{
    aCapturer->Init(settings);
}

void rtpStreamServer::stop()
{
    aCapturer->StopAudioCapture();
    tClient->stop();
}



int main(int argc, char* argv[])
{
    rtpStreamServer& serverInstance = rtpStreamServer::getInstance();
    bool result = serverInstance.start();

    (result) ? std::cout << "Test sucessufully " << std::endl : std::cerr << "Test failed - could not connect " << std::endl;

    char ch;
    std::cout << "Нажмите клавишу ESC для выхода...\n";
    do {
        ch = _getch();
    } while (ch != 27); // 27 - это ASCII-код для ESC

    std::cout << "Вы нажали клавишу ESC. Выход...\n";
    return 0;


};