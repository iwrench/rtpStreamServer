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
