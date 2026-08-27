#include "network/tcp_server.h"
#include "core/logger.h"

int main() {
    TCPServer::install_signal_handlers();
    TCPServer server(8080);

    if (!server.start()) {
        Logger::error("Failed to start server");
        return 1;
    }

    server.accept_loop();
    Logger::info("Server shut down cleanly");
    return 0;
}
