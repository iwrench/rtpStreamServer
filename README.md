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
Устанавливается через менеждер пакетов разработчика vcpkg 
.\vcpkg install boost-asio
.\vcpkg integrate install

### opus
Устанавливается через менеждер пакетов разработчика vcpkg 
.\vcpkg install opus
.\vcpkg integrate install

## Пример использования

#include "rtpStreamServer.h"
#include <conio.h>

int main(int argc, char* argv[])
{
    rtpStreamServer& serverInstance = rtpStreamServer::getInstance();
    char host[] = "example.com";
    char port[] = "8080";
    serverInstance.initRtpStreamServer(host, port);
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
