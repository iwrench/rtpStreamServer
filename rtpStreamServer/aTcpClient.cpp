#include "aTcpClient.h"

aTcpClient::aTcpClient(boost::asio::io_service& io_service) : block_size(NUM_SAMPLES_8000), io_service_(io_service), socket(io_service_), timer_(io_service), checkInterval_(5) {
}

void aTcpClient::connectToServer(const std::string& host, const std::string& port) {
  
    boost::asio::ip::tcp::resolver resolver(io_service_);
    boost::asio::ip::tcp::resolver::query query(host, port);
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    try {
        boost::asio::connect(socket, endpoint_iterator);
    }
    catch (...) {
        std::cerr << "Could not connect !" << std::endl;
    }
}

void aTcpClient::setCheckInterval(int seconds) {
    checkInterval_ = seconds;
}

size_t aTcpClient::sendData(const std::vector<unsigned char>& data) {
    size_t sendedBytesCount = 0;
    try {
        sendedBytesCount = boost::asio::write(socket, boost::asio::buffer(data));
    }
    catch (...) {
        sendedBytesCount = -1;
        std::cerr << "Could not connect !" << std::endl;
    }
    return sendedBytesCount;
}

size_t aTcpClient::sendData(boost::asio::ip::tcp::socket _socket, const std::vector<unsigned char>& data) {
    size_t sendedBytesCount = 0;
    try {
        sendedBytesCount = boost::asio::write(_socket, boost::asio::buffer(data));
    }
    catch (...) {
        sendedBytesCount = -1;
        std::cerr << "Could not connect !" << std::endl;
    }
    return sendedBytesCount;
}

void aTcpClient::checkConnection() {
    try {
        timer_.expires_from_now(boost::posix_time::seconds(checkInterval_));
        timer_.async_wait(boost::bind(&aTcpClient::checkConnection, this));
        char buff[1];
        size_t len = boost::asio::read(socket, boost::asio::buffer(buff), boost::asio::transfer_exactly(1));
        if (len == 0) {
            socket.close();
        }
    }
    catch (...) {
        std::cerr << "Could not connect !" << std::endl;
    }
}

bool aTcpClient::isConnect() {
    try {
        boost::asio::streambuf buffer;
        boost::asio::read(socket, buffer, boost::asio::transfer_exactly(0));
        return true;
    }
    catch (const boost::system::system_error&) {
        return false;
    }
}

void aTcpClient::listenTcpData(int nSamplesPerSec) {

    block_size = (nSamplesPerSec == 8000) ? NUM_SAMPLES_8000 : (nSamplesPerSec == 16000) ? NUM_SAMPLES_16000 : NUM_SAMPLES_48000;

    while (isConnect()) {
        std::vector<unsigned char> buffer(block_size);
        try {
            size_t len = socket.read_some(boost::asio::buffer(buffer));
            if (len > 0) {
                if (onReceiveCallback_) {
                    buffer.resize(len);
                    std::vector<boost::asio::detail::buffered_stream_storage::byte_type> mdata(buffer.begin(), buffer.end());
                    onReceiveCallback_(mdata);
                }
                else {
                    std::cerr << "onReceiveCallback_ don't have implementation !" << std::endl;
                }
            }
        }
        catch (const boost::system::system_error& e) {
            std::cerr << "System error: " << e.what() << std::endl;
            break;
        }
        catch (const std::exception& e) {
            std::cerr << "Exception: " << e.what() << std::endl;
            break;
        }

    }
}


void aTcpClient::stop() {
    socket.close();
}


void aTcpClient::convertData(const unsigned char* rData, size_t length, std::vector<boost::asio::detail::buffered_stream_storage::byte_type>& data) {
    // Очистить вектор перед заполнением
    data.clear();

    // Заполнить вектор данными из rData
    data.assign(rData, rData + length);
}

std::vector<boost::asio::detail::buffered_stream_storage::byte_type> aTcpClient::convertData(const signed char* data, std::size_t dataSize) {
    std::vector<boost::asio::detail::buffered_stream_storage::byte_type> mData(data, data + dataSize);
    return mData;
}

void  aTcpClient::convertData(const unsigned char* lpWaveBuf, std::size_t dataSize, signed short* dbits) {
    for (std::size_t i = 0; i < dataSize; i += 2) {
        dbits[i / 2] = static_cast<signed short>((lpWaveBuf[i + 1] << 8) | lpWaveBuf[i]);
    }
}

boost::asio::ip::tcp::socket& aTcpClient::getSocket() {
    return socket;
}

void aTcpClient::setSocket(boost::asio::ip::tcp::socket& newSocket) {
    socket = std::move(newSocket);
}
