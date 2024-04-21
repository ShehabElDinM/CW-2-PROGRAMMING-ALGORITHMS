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

int main() {
    int server_socket;
    struct sockaddr_in server_info;

    // Create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        Log("Socket creation failed");
        return 1;
    }

    // Server address configuration
    server_info.sin_family = AF_INET;
    server_info.sin_addr.s_addr = INADDR_ANY;
    server_info.sin_port = htons(9999);

    // Bind server socket
    if (bind(server_socket, (struct sockaddr *)&server_info, sizeof(server_info)) < 0) {
        Log("Binding failed");
        return 1;
    }

    //  Start listening on port 9999
    listen(server_socket, 5);
    Log("Server listening on port 9999");

    while (true) {
        int user_socket;
        struct sockaddr_in user_address;
        socklen_t user_addrlen = sizeof(user_address);

        // Accept incoming connection
        user_socket = accept(server_socket, (struct sockaddr *)&user_address, &user_addrlen);
        if (user_socket < 0) {
            Log("Acceptance failed");
            continue;
        }

        // Check if maximum number of clients reached
        if (num_clients >= NUM_USERS) {
            Log("Maximum number of clients reached. Connection rejected.");
            close(user_socket);
            continue;
        }

        // Finding a free slot for the user
        int free_index = -1;
        for (int i = 0; i < NUM_USERS; ++i) {
            if (users[i].socket == 0) {
                free_index = i;
                break;
            }
        }

        // Adding user information to an array
        if (free_index != -1) {
            users[free_index].socket = user_socket;
            users[free_index].authenticated = false;
            ++num_clients;

            // Get user IP address
            char user_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &user_address.sin_addr, user_ip, INET_ADDRSTRLEN);

            // Start a new thread to handle client connection
            thread user_thread(Handle_Client, user_socket, user_ip);
            user_thread.detach();
        } else {
            Log("Failed to find a free slot for the client.");
            close(user_socket);
        }
    }

    // Close server socket
    close(server_socket);
    return 0;
}
