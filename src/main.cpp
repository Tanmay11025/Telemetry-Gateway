#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

// Build and run from the project root:
// cmake -S . -B build && cmake --build build
// ./build/message_gateway [port] [backlog]

// argc is the number of command-line arguments.
// argv stores those arguments as strings: argv[0] is the program name.
int main(int argc, char* argv[]) {
    int port = 8080;
    int backlog = 128;

    // Read optional port and backlog values from argv.
    // Error handling rejects invalid, out-of-range, or extra arguments.
    try {
        if (argc > 1) {
            size_t characters_read = 0;
            port = stoi(argv[1], &characters_read);
            if (characters_read != string(argv[1]).length()) {
                throw invalid_argument("port contains non-numeric characters");
            }
        }

        if (argc > 2) {
            size_t characters_read = 0;
            backlog = stoi(argv[2], &characters_read);
            if (characters_read != string(argv[2]).length()) {
                throw invalid_argument("backlog contains non-numeric characters");
            }
        }

        if (argc > 3) {
            throw invalid_argument("too many arguments");
        }
    } catch (const invalid_argument& error) {
        cerr << "Invalid argument: " << error.what() << '\n';
        cerr << "Usage: " << argv[0] << " [port] [backlog]\n";
        return EXIT_FAILURE;
    } catch (const out_of_range&) {
        cerr << "Invalid argument: value is too large\n";
        cerr << "Usage: " << argv[0] << " [port] [backlog]\n";
        return EXIT_FAILURE;
    }

    if (port < 1 || port > 65535 || backlog < 1) {
        cerr << "Error: port must be between 1 and 65535, and backlog must be positive\n";
        return EXIT_FAILURE;
    }

    // socket() creates an endpoint for TCP communication.
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0) {
        cout << "Server socket allocation failed\n";
        return EXIT_FAILURE;
    }

    // setsockopt() allows the port to be reused after a restart.
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        cout << "setsockopt() failed: \n";
        close(server_fd);
        return EXIT_FAILURE;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(static_cast<std::uint16_t>(port));
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // bind() assigns the IP address and port to the socket.
    if (bind(server_fd, (struct sockaddr*)(&server_addr), sizeof(server_addr)) < 0) {
        cout << "bind() failed: " << '\n';
        close(server_fd);
        return EXIT_FAILURE;
    }

    // listen() puts the socket into server mode and queues new clients.
    if (listen(server_fd, backlog) < 0) {
        cout << "listen() failed: " << '\n';
        close(server_fd);
        return EXIT_FAILURE;
    }

    cout << "Listening on port " << port << " with backlog " << backlog << '\n';

    // Accept and handle one client at a time.
    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_addr_len = sizeof(client_addr);

        // accept() waits for a client and returns a new client socket.
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_fd < 0) {
            cerr << "accept() failed: " << '\n';
            continue;
        }

        char buffer[4096];

        while (true) {
            // recv() reads data sent by the connected client.
            ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

            if (bytes_read == 0) {
                break;
            }
            if (bytes_read < 0) {
                std::cerr << "recv() failed: " << '\n';
                break;
            }

            // write() sends the received data back to the client.
            ssize_t bytes_written = 0;
            while (bytes_written < bytes_read) {
                ssize_t result = write(client_fd, buffer + bytes_written, bytes_read - bytes_written);
                if (result < 0) {
                    cout << "write() failed: \n";
                    break;
                }
                bytes_written += result;
            }

            if (bytes_written < bytes_read) {
                break;
            }
        }

        // close() releases the client socket when the session ends.
        close(client_fd);
    }

    // Close the listening socket when the server stops.
    close(server_fd);
    return 0;
}
