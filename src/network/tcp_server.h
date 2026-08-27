#pragma once

#include <atomic>
#include <csignal>
#include <string>

class TCPServer {
public:
    explicit TCPServer(int port, int backlog = 128);
    ~TCPServer();

    TCPServer(const TCPServer&) = delete;
    TCPServer& operator=(const TCPServer&) = delete;

    bool start();
    void accept_loop();
    void stop();

    int port() const noexcept { return port_; }
    int listen_fd() const noexcept { return listen_fd_; }
    bool is_running() const noexcept { return running_; }

    static void install_signal_handlers();

private:
    static void on_signal(int sig);

    int port_;
    int backlog_;
    int listen_fd_;
    bool running_;
    static std::atomic<bool> shutdown_requested_;
};
