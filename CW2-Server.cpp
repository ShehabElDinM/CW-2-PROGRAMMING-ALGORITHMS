#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <fstream>
#include <sstream>
#include <thread>


using namespace std;

using namespace std;

constexpr int NUM_USERS = 100;

struct User_Info {
    int socket;
    bool authenticated;
};

User_Info users[NUM_USERS];

int num_clients = 0;

void Log(const string& message) {
    cout << message << endl;
}

string Encryption(const string& message, int shift) {
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

string Decryption(const string& message, int shift) {
    return Encryption(message, 26 - shift);
}

void Broadcast_M(const string& message, int sender_socket) {

    string encrypted_message = Encryption(message, 3);

    ofstream outfile("User_Messages.txt", ios::app); 
    outfile << encrypted_message << endl;

    outfile.close();
    for (int i = 0; i < NUM_USERS; ++i) {
        if (users[i].socket != 0 && users[i].socket != sender_socket) {
            send(users[i].socket, encrypted_message.c_str(), encrypted_message.length(), 0);
        }
    }
}

bool Authenticate_C(int user_socket) {
    char buffer[1024] = {0};

    if (recv(user_socket, buffer, 1024, 0) <= 0) {
        return false;
    }
    string received_credentials(buffer);

    string decrypted_credentials = Decryption(received_credentials, 3);

    ifstream infile("User_Credential.txt");
    string line;
    while (getline(infile, line)) {
        istringstream iss(line);
        string stored_username;
        string stored_password;
        if (!(iss >> stored_username >> stored_password)) { break; }
        if (decrypted_credentials == (stored_username + " " + stored_password)) {
            return true;
        }
    }
    return false;
}

void Handle_Client(int user_socket, const string& user_address) {
    Log("Connection from: " + user_address);


    if (!Authenticate_C(user_socket)) {
        Log("Authentication failed for client: " + user_address);
        close(user_socket);
        return;
    }

    Log("Authentication successful for client: " + user_address);

    char buffer[1024] = {0};
    while (true) {
        int valread = recv(user_socket, buffer, 1024, 0);
        if (valread <= 0) {
            break;
        }
        string message(buffer);
        Broadcast_M(message, user_socket);
        memset(buffer, 0, sizeof(buffer));
    }

    close(user_socket);
    Log("Connection with client: " + user_address + " closed");
}
