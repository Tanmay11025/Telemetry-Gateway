#include "tcp_server.h"
#include "core/logger.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>

std::atomic<bool> TCPServer::shutdown_requested_{false};

TCPServer::TCPServer(int port, int backlog)
    : port_(port), backlog_(backlog), listen_fd_(-1), running_(false) {}

TCPServer::~TCPServer() {
    if (listen_fd_ >= 0) {
        close(listen_fd_);
        listen_fd_ = -1;
    }
}

bool TCPServer::start() {
    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ < 0) {
        Logger::error("socket() failed: %s", strerror(errno));
        return false;
    }

    int opt = 1;
    setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port_));
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        Logger::error("bind() failed: %s", strerror(errno));
        close(listen_fd_);
        listen_fd_ = -1;
        return false;
    }

    if (listen(listen_fd_, backlog_) < 0) {
        Logger::error("listen() failed: %s", strerror(errno));
        close(listen_fd_);
        listen_fd_ = -1;
        return false;
    }

    running_ = true;
    Logger::info("Listening on 0.0.0.0:%d (backlog=%d)", port_, backlog_);
    return true;
}

void TCPServer::accept_loop() {
    while (running_ && !shutdown_requested_.load(std::memory_order_relaxed)) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(listen_fd_, reinterpret_cast<sockaddr*>(&client_addr), &client_len);

        if (client_fd < 0) {
            if (errno == EINTR) continue;
            Logger::error("accept() failed: %s", strerror(errno));
            continue;
        }

        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str));
        Logger::info("Client connected: fd=%d from %s:%d",
                     client_fd, ip_str, ntohs(client_addr.sin_port));

        char buffer[4096];
        while (running_ && !shutdown_requested_.load(std::memory_order_relaxed)) {
            ssize_t bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
            if (bytes == 0) {
                Logger::info("Client disconnected: fd=%d", client_fd);
                break;
            }
            if (bytes < 0) {
                if (errno == EINTR) continue;
                Logger::error("recv() failed: %s", strerror(errno));
                break;
            }

            buffer[bytes] = '\0';
            Logger::info("Received %zd bytes from fd=%d: %.*s",
                         bytes, client_fd, static_cast<int>(bytes), buffer);
            send(client_fd, buffer, static_cast<size_t>(bytes), 0);
        }

        close(client_fd);
    }

    running_ = false;
}

void TCPServer::stop() {
    running_ = false;
}

void TCPServer::on_signal(int /*sig*/) {
    shutdown_requested_.store(true, std::memory_order_relaxed);
}

void TCPServer::install_signal_handlers() {
    struct sigaction sa{};
    sa.sa_handler = &TCPServer::on_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
}
