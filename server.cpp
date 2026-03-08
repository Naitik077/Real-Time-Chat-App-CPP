server.cpp#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>

using namespace std;

vector<int> active_clients;
mutex clients_mutex;

void broadcast_message(const string& message, int sender_socket) {
    lock_guard<mutex> lock(clients_mutex);
    for (int client_fd : active_clients) {
        if (client_fd != sender_socket) {
            send(client_fd, message.c_str(), message.length(), 0);
        }
    }
}

void handle_client(int client_socket) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            lock_guard<mutex> lock(clients_mutex);
            active_clients.erase(remove(active_clients.begin(), active_clients.end(), client_socket), active_clients.end());
            close(client_socket);
            break;
        }
        string msg(buffer);
        broadcast_message(msg, client_socket);
    }
}

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    if (::bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) return 1;
    if (listen(server_socket, 10) < 0) return 1;

    cout << "Server Started\n";

    while (true) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);

        {
            lock_guard<mutex> lock(clients_mutex);
            active_clients.push_back(client_socket);
        }
        string welcome_msg = "Welcome!\n";
        send(client_socket, welcome_msg.c_str(), welcome_msg.length(), 0);

        thread client_thread(handle_client, client_socket);
        client_thread.detach();
    }
    return 0;
}
