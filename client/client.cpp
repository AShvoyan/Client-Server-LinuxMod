#include <iostream>
#include <cstring>
#include <thread>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdlib>

using namespace std;

const int MAX_BUFFER_SIZE = 8;

void handle_response(int server_socket) {
    uint64_t result;
    int bytes_received = recv(server_socket, &result, sizeof(result), 0);
    if (bytes_received == -1 || bytes_received == 0) {
        close(server_socket);
        return;
    }
    result = be64toh(result); // Convert result from network byte order
    cout << "Result: " << result << endl;
}

void connect_to_server(const char* ip_address, int port) {
    while (true) {
        int client_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (client_socket == -1) {
            perror("socket");
            exit(1);
        }
        sockaddr_in server_address;
        memset(&server_address, 0, sizeof(server_address));
        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = inet_addr(ip_address);
        server_address.sin_port = htons(port);
        if (connect(client_socket, (sockaddr*)&server_address, sizeof(server_address)) == -1) {
            perror("connect");
            close(client_socket);
            continue;
        }
        int m;
        cout << "Enter a number: ";
        cin >> m;
        m = htonl(m);
        send(client_socket, &m, sizeof(m), 0);
        thread t(handle_response, client_socket);
        t.detach();
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <ip_address> <port>" << endl;
        exit(1);
    }
    const char* ip_address = argv[1];
    int port = atoi(argv[2]);
    connect_to_server(ip_address, port);
    return 0;
}
