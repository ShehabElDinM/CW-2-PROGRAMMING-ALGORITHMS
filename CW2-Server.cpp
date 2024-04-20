#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


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