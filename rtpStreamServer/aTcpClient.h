#pragma once
#ifndef ATCPCLIENT_H
#define ATCPCLIENT_H

#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <boost/bind.hpp>

#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <vector>
#include <functional>
#include <iostream>

#define NUM_SAMPLES_8000 960
#define NUM_SAMPLES_16000 1920
#define NUM_SAMPLES_48000 5760

class aTcpClient {
public:
    aTcpClient(boost::asio::io_service& io_service);
    void connectToServer(const std::string& host, const std::string& port);

    void setCheckInterval(int seconds);
    size_t sendData(const std::vector<unsigned char>& data);
    size_t sendData(boost::asio::ip::tcp::socket _socket, const std::vector<unsigned char>& data);
    void checkConnection();
    bool isConnect();
    void listenTcpData(int nSamplesPerSec);
    void stop();

    void convertData(const unsigned char* rData, size_t length, std::vector<boost::asio::detail::buffered_stream_storage::byte_type>& data);
    std::vector<boost::asio::detail::buffered_stream_storage::byte_type> convertData(const signed char* data, std::size_t dataSize);
    void convertData(const unsigned char* lpWaveBuf, std::size_t dataSize, signed short* dbits);

    boost::asio::ip::tcp::socket& getSocket();
    void setSocket(boost::asio::ip::tcp::socket& newSocket);

    std::function<void(const std::vector<boost::asio::detail::buffered_stream_storage::byte_type>&)> onReceiveCallback_;

private:
    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::socket socket;
    boost::asio::deadline_timer timer_;
    int checkInterval_;
    int block_size;
};

#endif // ATCPCLIENT_H


