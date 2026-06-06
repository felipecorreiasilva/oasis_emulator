#include "socket.hpp"
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
typedef struct pollfd WSAPOLLFD;
#define WSAPoll poll
#endif

std::unordered_map<socket_t, SessionData*> session_map;
bool server_is_running = true;

// Ponteiro global que vai segurar a função de leitura do servidor atual
int (*default_parse_func)(int fd, SessionData& session) = nullptr;

bool oasis_socket_init() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[Network] Falha critica no WSASend/Recv do Windows!" << std::endl;
        return false;
    }
#endif
    session_map.clear();
    return true;
}

void set_nonblocking(socket_t fd) {
#ifdef _WIN32
    unsigned long mode = 1;
    ioctlsocket(fd, FIONBIO, &mode);
#else
    fcntl(fd, F_SETFL, O_NONBLOCK);
#endif
}

socket_t make_listen_bind(const std::string& ip, uint16_t port) {
    socket_t listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_fd == INVALID_SOCKET) return INVALID_SOCKET;

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    int buffer_size = 256 * 1024;
    setsockopt(listen_fd, SOL_SOCKET, SO_RCVBUF, (const char*)&buffer_size, sizeof(buffer_size));
    setsockopt(listen_fd, SOL_SOCKET, SO_SNDBUF, (const char*)&buffer_size, sizeof(buffer_size));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (ip.empty() || ip == "ANY") {
        addr.sin_addr.s_addr = INADDR_ANY;
    }
    else {
#ifdef _WIN32
        inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);
#else
        addr.sin_addr.s_addr = inet_addr(ip.c_str());
#endif
    }

    if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) return INVALID_SOCKET;
    if (listen(listen_fd, SOMAXCONN) == SOCKET_ERROR) return INVALID_SOCKET;

    set_nonblocking(listen_fd);
    return listen_fd;
}

void session_write(SessionData& session, const void* data, size_t size) {
    const uint8_t* byte_ptr = reinterpret_cast<const uint8_t*>(data);
    session.wdata.insert(session.wdata.end(), byte_ptr, byte_ptr + size);
}

void session_skip_read(SessionData& session, size_t size) {
    if (size >= session.rdata.size()) {
        session.rdata.clear();
    }
    else {
        session.rdata.erase(session.rdata.begin(), session.rdata.begin() + size);
    }
}

void close_session(SessionData* session) {
    if (!session) return;
    std::cout << "[Network] Fechando conexao de: " << session->ip << " (FD: " << session->fd << ")" << std::endl;
#ifdef _WIN32
    closesocket(session->fd);
#else
    close(session->fd);
#endif
    session_map.erase(session->fd);
    delete session;
}

void do_sockets_main_loop(socket_t listen_fd) {
    std::vector<WSAPOLLFD> poll_fds;

    while (server_is_running) {
        poll_fds.clear();

        WSAPOLLFD listen_pfd{};
        listen_pfd.fd = listen_fd;
        listen_pfd.events = POLLRDNORM;
        poll_fds.push_back(listen_pfd);

        for (const auto& [fd, session] : session_map) {
            WSAPOLLFD pfd{};
            pfd.fd = fd;
            pfd.events = POLLRDNORM;
            if (!session->wdata.empty()) {
                pfd.events |= POLLWRNORM;
            }
            poll_fds.push_back(pfd);
        }

#ifdef _WIN32
        int ret = WSAPoll(poll_fds.data(), static_cast<ULONG>(poll_fds.size()), 1);
#else
        int ret = WSAPoll(poll_fds.data(), poll_fds.size(), 1);
#endif
        if (ret < 0) continue;

        for (size_t i = 0; i < poll_fds.size(); ++i) {
            if (poll_fds[i].revents == 0) continue;

            // Aceitar conexões (Novos Clientes)
            if (poll_fds[i].fd == listen_fd) {
                if (poll_fds[i].revents & POLLRDNORM) {
                    sockaddr_in client_addr{};
                    int addr_len = sizeof(client_addr);
                    socket_t client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &addr_len);

                    if (client_fd != INVALID_SOCKET) {
                        set_nonblocking(client_fd);

                        SessionData* session = new SessionData();
                        session->fd = client_fd;

                        char ip_str[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, INET_ADDRSTRLEN);
                        session->ip = ip_str;
                        session->port = ntohs(client_addr.sin_port);
                        session->parse_func = default_parse_func; // injeta o parse_login aqui

                        session_map[client_fd] = session;
                        std::cout << "[Network] Cliente conectado com sucesso! IP: " << session->ip << " (FD: " << client_fd << ")" << std::endl;
                    }
                }
                continue;
            }

            socket_t current_fd = poll_fds[i].fd;
            auto it = session_map.find(current_fd);
            if (it == session_map.end()) continue;
            SessionData* session = it->second;

            // Evento de Leitura (Receber dados do Cliente)
            if (poll_fds[i].revents & (POLLRDNORM | POLLHUP)) {
                uint8_t buffer[8192];
                int bytes_recv = recv(current_fd, reinterpret_cast<char*>(buffer), sizeof(buffer), 0);

                if (bytes_recv <= 0) {
                    close_session(session);
                    continue;
                }
                else {
                    // Joga no buffer FIFO de entrada
                    session->rdata.insert(session->rdata.end(), buffer, buffer + bytes_recv);

                    // Chama a lógica de ler os pacotes (Parser)
                    if (session->parse_func) {
                        int processed = 0;
                        while ((processed = session->parse_func(static_cast<int>(current_fd), *session)) > 0) {
                            session_skip_read(*session, static_cast<size_t>(processed));
                        }
                        if (processed < 0) {
                            close_session(session);
                            continue;
                        }
                    }
                }
            }

            // Evento de Escrita (Enviar respostas guardadas no wdata)
            if (poll_fds[i].revents & POLLWRNORM) {
                if (!session->wdata.empty()) {
                    int bytes_sent = send(current_fd, reinterpret_cast<const char*>(session->wdata.data()), static_cast<int>(session->wdata.size()), 0);
                    if (bytes_sent > 0) {
                        session->wdata.erase(session->wdata.begin(), session->wdata.begin() + bytes_sent);
                    }
                    else if (bytes_sent == SOCKET_ERROR) {
                        close_session(session);
                        continue;
                    }
                }
            }
        }
    }
}

void oasis_socket_shutdown() {
    server_is_running = false;
    for (auto& [fd, session] : session_map) {
        if (session) {
#ifdef _WIN32
            closesocket(fd);
#else
            close(fd);
#endif
            delete session;
        }
    }
    session_map.clear();
#ifdef _WIN32
    WSACleanup();
#endif
}