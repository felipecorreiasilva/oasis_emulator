#ifndef OASIS_SOCKET_HPP
#define OASIS_SOCKET_HPP

#include <vector>
#include <string>
#include <memory>

#ifdef _WIN32
#include <winsock2.h>
typedef SOCKET socket_t;
#else
typedef int socket_t;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#endif

// Tamanhos de buffers de alta performance (64KB de buffering circular/FIFO)
const size_t BUFFER_MAX_SIZE = 65536;

struct SessionData {
    socket_t fd;
    std::string ip;
    uint16_t port;

    // FIFO Buffers usando vetores pré-alocados para evitar reallocs em runtime
    std::vector<uint8_t> rdata;
    std::vector<uint8_t> wdata;

    // Ponteiro de função para o parser do pacote (Login/Char/Map)
    int (*parse_func)(int fd, SessionData& session);
};

// Funções de inicialização do ecossistema de rede
bool oasis_socket_init();
void oasis_socket_shutdown();

// Gerenciamento de portas e conexões
socket_t make_listen_bind(const std::string& ip, uint16_t port);
void set_nonblocking(socket_t fd);

// O Motor de Eventos Principal (Equivalente ao loop da libuv/epoll do rAthena)
void do_sockets_main_loop(socket_t listen_fd);

// Manipulação rápida de Buffers (RFIFO / WFIFO)
void session_write(SessionData& session, const void* data, size_t size);
void session_skip_read(SessionData& session, size_t size);
void close_session(SessionData* session);

// ESSA LINHA PRECISA ESTAR AQUI DENTRO DO SOCKET.HPP:
extern int (*default_parse_func)(int fd, SessionData& session);

#endif