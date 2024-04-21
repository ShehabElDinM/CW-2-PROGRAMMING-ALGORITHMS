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
