#include <iostream>
#include <cstring>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdlib>
#include <cmath>

using namespace std;

const int MAX_CONNECTIONS = 10;
const int MAX_BUFFER_SIZE = 4;
const int MAX_PRIME_SIZE = 8;

bool isPrime(int number) {
    if (number <= 1) {
        return false;
    }

    for (int i = 2; i * i <= number; ++i) {
        if (number % i == 0) {
            return false;
        }
    }

    return true;
}

queue<int> connection_queue;
mutex connection_mutex;
condition_variable connection_cv;

void compute_mth_prime(int client_socket, int m) {
    int count = 0;
    int i = 2;
    while (count < m) {
        if (isPrime(i)) {
            count++;
        }
        i++;
    }
    uint64_t result = i - 1;
    result = htobe64(result); // Convert result to network byte order
    send(client_socket, &result, sizeof(result), 0);
    close(client_socket);
}

void handle_connection() {
    while (true) {
        unique_lock<mutex> lock(connection_mutex);
        connection_cv.wait(lock, [] { return !connection_queue.empty(); });
        int client_socket = connection_queue.front();
        connection_queue.pop();
        lock.unlock();
        char buffer[MAX_BUFFER_SIZE];
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received == -1 || bytes_received == 0) {
            close(client_socket);
            continue;
        }
        int m = ntohl(*(int*)buffer);
        thread t(compute_mth_prime, client_socket, m);
        t.detach();
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <port>" << endl;
        exit(1);
    }
    int port = atoi(argv[1]);
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("socket");
        exit(1);
    }
    sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(port);
    if (bind(server_socket, (sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("bind");
        exit(1);
    }
    if (listen(server_socket, MAX_CONNECTIONS) == -1) {
        perror("listen");
        exit(1);
    }
    thread pool[MAX_CONNECTIONS];
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        pool[i] = thread(handle_connection);
    }
    while (true) {
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == -1) {
            perror("accept");
            continue;
        }
        unique_lock<mutex> lock(connection_mutex);
        connection_queue.push(client_socket);
        lock.unlock();
        connection_cv.notify_one();
    }
    return 0;
}
