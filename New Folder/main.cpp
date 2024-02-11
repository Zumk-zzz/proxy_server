#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

constexpr int MAX_EVENTS = 100;
constexpr int BUFFER_SIZE = 1024;

// Функция для записи SQL запросов в файл
void WriteToLogFile(const std::string& sql_query) {
    std::ofstream logfile("sql_queries.log", std::ios::app);
    if (logfile.is_open()) {
        logfile << sql_query << std::endl;
    } else {
        std::cerr << "Unable to open log file for writing" << std::endl;
    }
}

int main() {
    int server_fd, client_fd, epoll_fd;
    struct epoll_event event, events[MAX_EVENTS];

    // Создание сокета для сервера
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return 1;
    }

    // Установка опции SO_REUSEADDR для переиспользования порта
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        return 1;
    }
    // Устанавливаем параметры нашего прокси сервера (Принимать подключение со всехх доступных адрессов, 9999 порт)
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(9999); 

    // Привязка сокета к адресу и порту
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return 1;
    }

    // Установка серверного сокета в режим прослушивания
    if (listen(server_fd, 10) < 0) {
        perror("listen failed");
        return 1;
    }

    // Создание epoll дескриптора
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1 failed");
        return 1;
    }

    // Добавление серверного сокета в epoll
    event.events = EPOLLIN;
    event.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1) {
        perror("epoll_ctl: server_fd");
        return 1;
    }

    while (true) {
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (num_events == -1) {
            perror("epoll_wait failed");
            return 1;
        }

        for (int i = 0; i < num_events; ++i) {
            if (events[i].data.fd == server_fd) {
                // Принимаем новое соединение
                if ((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                    perror("accept failed");
                    continue;
                }

                // Устанавливаем неблокирующий режим для клиентского сокета
                int flags = fcntl(client_fd, F_GETFL, 0);
                fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

                // Добавляем клиентский сокет в epoll
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = client_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
                    perror("epoll_ctl: client_fd");
                    return 1;
                }
            } else {
                // Читаем данные из клиентского сокета
                char buffer[BUFFER_SIZE];
                int bytes_read = read(events[i].data.fd, buffer, BUFFER_SIZE);

                if (bytes_read <= 0) {
                    // Ошибка чтения или клиент закрыл соединение
                    close(events[i].data.fd);
                } else {
                    // Парсим SQL запросы из принятых данных и записываем в файл
                    std::string data(buffer, bytes_read);
                    size_t start_pos = 0;
                    size_t end_pos;
                    while ((end_pos = data.find('\n', start_pos)) != std::string::npos) {
                        std::string sql_query = data.substr(start_pos, end_pos - start_pos);
                        WriteToLogFile(sql_query);
                        start_pos = end_pos + 1;
                    }
                }
            }
        }
    }

    return 0;
}
