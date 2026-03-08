#include <iostream>
#include <string>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

using namespace std;

void receive_messages(int socket_fd) {
      char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(socket_fd, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) exit(0);
        cout << "\r" << buffer << "\nYou: " << flush;
    }
}

int main() {
      string username;
    cout << "Enter username: ";
    getline(cin, username);

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));

    thread recv_thread(receive_messages, client_socket);
    recv_thread.detach();

    string message;
    while (true) {
        getline(cin, message);
        if (message == "exit") break;
        if (!message.empty()) {
            string formatted_message = username + ": " + message;
            send(client_socket, formatted_message.c_str(), formatted_message.length(), 0);
        }
    }
    close(client_socket);
    return 0;
}
