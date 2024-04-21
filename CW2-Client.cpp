#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <sstream>
#include <fstream>
#include <pthread.h>

using namespace std;

void log(const string& message) {
    cout << message << endl;
}

string Encrypt(const string& message, int shift) {
    string result = "";
    for (char c : message) {
        if (isalpha(c)) {
            char base = isupper(c) ? 'A' : 'a';
            result += (c - base + shift) % 26 + base;
        } else {
            result += c;
        }
    }
    return result;
}

string Decrypt(const string& message, int shift) {
    return Encrypt(message, 26 - shift);
}

bool Auth(int server_socket) {
    char username[1024];
    char password[1024];

    cout << "Enter username: ";
    cin.getline(username, sizeof(username));

    cout << "Enter password: ";
    cin.getline(password, sizeof(password));

    string credentials = string(username) + " " + string(password);

    string encrypted_credentials = Encrypt(credentials, 3);

    if (send(server_socket, encrypted_credentials.c_str(), encrypted_credentials.length(), 0) == -1) {
        log("Send failed");
        return false;
    }

    return true;
}

void* Receive(void* arg) {
    int server_socket = *((int*)arg);
    char buffer[1024] = {0};
    while (true) {
        int valread = recv(server_socket, buffer, sizeof(buffer), 0);
        if (valread <= 0) {
            log("Receive failed");
            break;
        }
        string decrypted_message = Decrypt(buffer, 3);
        cout << "server: " << decrypted_message << endl;

        memset(buffer, 0, sizeof(buffer));
    }
    return NULL;
}

int main() {
    int client_socket;
    struct sockaddr_in server_info;

    // Create client socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        log("Socket creation failed");
        return 1;
    }

    // Server address configuration
    server_info.sin_family = AF_INET;
    server_info.sin_port = htons(9999);

    if (inet_pton(AF_INET, "127.0.0.1", &server_info.sin_addr) <= 0) {
        log("Invalid address");
        return 1;
    }

    // Establishing a connection with the Server
    if (connect(client_socket, (struct sockaddr *)&server_info, sizeof(server_info)) < 0) {
        log("Connection failed");
        return 1;
    }

    // Authenticating credentials with the server
    if (!Auth(client_socket)) {
        log("Authentication failed");
        close(client_socket);
        return 1;
    }

    log("Authentication successful");

    // Starting a new thread to receive messages from the server
    pthread_t receive_thread;
    if (pthread_create(&receive_thread, NULL, Receive, (void*)&client_socket) != 0) {
        log("Failed to create receive thread");
        close(client_socket);
        return 1;
    }

    char buffer[1024] = {0};
    while (true) {
        string message;

        // Read a line of input from the user
        getline(cin, message); 
        if (message.empty()) {
            continue;
        }

        // Send message to server
        if (send(client_socket, message.c_str(), message.length(), 0) == -1) {
            log("Send failed");
            break;
        }
    }

    // Joining the receive thread
    pthread_join(receive_thread, NULL);

    // Close client socket
    close(client_socket);
    return 0;
}